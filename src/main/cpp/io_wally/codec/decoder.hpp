#pragma once

#include <cstdint>
#include <cstring>
#include <cmath>
#include <string>
#include <tuple>
#include <stdexcept>
#include <memory>

#include <boost/optional.hpp>
#include <boost/system/error_code.hpp>

#include "io_wally/error/protocol.hpp"
#include "io_wally/protocol/common.hpp"

namespace io_wally
{
    /// \brief Namespace for grouping types and functions related to decoding \c mqtt_packets.
    ///
    /// Most important classes in this namespace are arguably
    ///
    ///  - \c header_decoder and
    ///  - \c mqtt_packet_decoder
    ///
    /// \note       The grunt work of decoding an \c mqtt_packet's on the wire representation is done by several
    ///             implementations of \c packet_body_decoder, one for each concrete \c mqtt_packet.
    namespace decoder
    {
        struct frame final
        {
           public:  // static
            using const_iterator_t = std::vector<uint8_t>::const_iterator;

           public:
            frame( const uint8_t type_and_flags, const const_iterator_t begin, const const_iterator_t end )
                : type_and_flags{type_and_flags}, begin{begin}, end{end}
            {
            }

            std::size_t remaining_length( ) const
            {
                return std::distance( begin, end );
            }

            protocol::packet::Type type( ) const
            {
                return protocol::packet::type_of( type_and_flags );
            }

           public:
            const uint8_t type_and_flags;
            const const_iterator_t begin;
            const const_iterator_t end;
        };  // struct frame

        class frame_reader final
        {
           private:  // static
            static constexpr const uint8_t MSB_MASK = 0x80;

           public:
            frame_reader( std::vector<uint8_t>& buffer ) : buffer_{buffer}
            {
            }

            std::size_t operator( )( const boost::system::error_code& error, std::size_t bytes_transferred )
            {
                assert( bytes_transferred <= buffer_.size( ) );
                if ( error )
                {
                    return 0;
                }

                const auto rlen_hlen = decode_remaining_length( bytes_transferred );
                if ( !rlen_hlen )
                {
                    // It should happen exceedingly rarely that we do not receive enough bytes to decode a packet's
                    // remaining length header field. If it happens, we proceed very cautiously and tell asio to only
                    // read one more byte.
                    return 1;
                }
                const auto rlen = ( *rlen_hlen ).first;
                const auto hlen = ( *rlen_hlen ).second;
                const auto len = hlen + rlen;
                const auto remaining = len - bytes_transferred;

                // Expand buffer to accommodate entire frame if necessary
                if ( len > buffer_.size( ) )
                {
                    buffer_.insert( std::end( buffer_ ), len - buffer_.size( ), 0x00 );
                }
                assert( len <= buffer_.size( ) );

                if ( remaining == 0 )
                {
                    // Need to use emplace() here to preserve frame constness
                    frame_.emplace( frame{buffer_[0], std::begin( buffer_ ) + hlen, std::begin( buffer_ ) + len} );
                }

                return remaining;
            }

            boost::optional<const frame> get_frame( ) const
            {
                return frame_;
            }

            void reset( )
            {
                frame_ = boost::none;
            }

           private:
            const boost::optional<std::pair<std::size_t, std::size_t>> decode_remaining_length(
                std::size_t bytes_transferred ) const
            {
                if ( bytes_transferred < 2 )
                {
                    return boost::none;
                }

                auto rlen = std::size_t{0};
                for ( std::vector<uint8_t>::size_type i = 1; i < bytes_transferred; ++i )
                {
                    if ( i > 4 )
                    {
                        throw error::malformed_mqtt_packet( "Encoded remaining length exceeds maximum allowed value" );
                    }
                    const auto current_byte = buffer_[i];
                    const auto multiplier = pow( 128, i - 1 );
                    rlen += ( current_byte & ~MSB_MASK ) * multiplier;
                    if ( ( current_byte & MSB_MASK ) == 0 )
                    {
                        return std::make_pair( rlen, i + 1 );
                    }
                }

                return boost::none;
            }

           private:
            std::vector<uint8_t>& buffer_;
            boost::optional<const frame> frame_ = boost::none;
        };  // class frame_reader

        /// \brief Parse a 8 bit wide unsigned int in the supplied buffer into a \c packet::QoS.
        ///
        /// Start decoding at \c uint8_start, interpreting the supplied buffer as a 8 bit unsigned int. Stop decoding
        /// when reaching the end of unsigned int to parse, or \c buf_end, whichever comes first. Transform parsed
        /// unsigned int into \c packet::QoS and assign the result to \c parsed_qos, or throw
        /// \c error::malformed_mqtt_packet if input is malformed.
        ///
        /// \param uint8_start      Start of buffer to parse.
        /// \param buf_end          End of entire packet buffer, NOT end of uint16_t buffer (although that would
        ///                         work, too). Needed for range checks.
        /// \param parsed_qos       A pointer to the parsed \c packet::QoS (out parameter), will never be \c nullptr.
        /// \return                 The updated \c InputIterator \c uint8_start, thus telling the caller where to
        ///                         continue decoding.
        /// \throws error::malformed_mqtt_packet        If decoding fails due to malformed input.
        ///
        /// \pre        \c uint16_start initially points to the first byte of a two byte sequence in big endian.
        /// \pre        \c buf_end points immediately past the last byte in a buffer representing an
        ///             \c mqtt_packet's on the wire format.
        /// \post       The \c InputIterator returned points to the first unconsumed byte.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718016
        template <typename InputIterator>
        inline std::pair<InputIterator, uint16_t> decode_uint16( InputIterator uint16_start,
                                                                 const InputIterator buf_end )
        {
            // We need at least two bytes for encoding a uint16_t
            if ( uint16_start + 2 > buf_end )
            {
                throw error::malformed_mqtt_packet( "Encoding a 16 bit unsigned int needs at least two bytes" );
            }

            const uint8_t msb = *uint16_start++;
            const uint8_t lsb = *uint16_start++;
            const uint16_t parsed_uint16 = ( msb << 8 ) + lsb;

            return std::make_pair( uint16_start, parsed_uint16 );
        }

        /// \brief Parse a 16 bit wide unsigned int in the supplied buffer.
        ///
        /// Start decoding at \c uint16_start, interpreting the supplied buffer as a 16 bit unsigned int in big
        /// endian byte order, as mandated by MQTT. Stop decoding when reaching the end of unsigned int to parse,
        /// or \c buf_end, whichever comes first. Return the updated input iterator, or throw
        /// \c error::malformed_mqtt_packet if input is malformed.
        ///
        /// \note           In MQTT 3.1.1 integers are encoded in big endian byte order.
        ///
        /// \param uint16_start     Start of buffer to parse.
        /// \param buf_end          End of entire packet buffer, NOT end of uint16_t buffer (although that would
        ///                         work, too). Needed for range checks.
        /// \param parsed_uint16    A pointer to the parsed integer (out parameter), will never be \c nullptr.
        /// \return                 The updated \c InputIterator \c uint16_start, thus telling the caller where to
        ///                         continue decoding.
        /// \throws error::malformed_mqtt_packet        If decoding fails due to malformed input.
        ///
        /// \pre        \c uint8_start initially points to an unsigned int, 8 bits, representing a \c packet::QoS
        /// \pre        \c buf_end points immediately past the last byte in a buffer representing an
        ///             \c mqtt_packet's on the wire format.
        /// \post       The \c InputIterator returned points to the first unconsumed byte.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718016
        template <typename InputIterator>
        inline InputIterator decode_qos( InputIterator uint8_start,
                                         const InputIterator buf_end,
                                         io_wally::protocol::packet::QoS* const parsed_qos )
        {
            using namespace io_wally::protocol;
            // We need at least one byte for encoding a packet::QoS
            if ( uint8_start + 1 > buf_end )
            {
                throw error::malformed_mqtt_packet( "Encoding QoS needs at least one byte" );
            }

            const uint8_t qos_bits = *uint8_start++;
            switch ( qos_bits )
            {
                case 0x00:
                    *parsed_qos = packet::QoS::AT_MOST_ONCE;
                    break;
                case 0x01:
                    *parsed_qos = packet::QoS::AT_LEAST_ONCE;
                    break;
                case 0x02:
                    *parsed_qos = packet::QoS::EXACTLY_ONCE;
                    break;
                default:
                    *parsed_qos = packet::QoS::RESERVED;
                    break;
            }

            return uint8_start;
        }  // decode_qos

        /// \brief Parse a UTF-8 string in the supplied buffer.
        ///
        /// Start decoding at \c string_start, interpreting the supplied buffer as an MQTT UTF-8 string. Stop
        /// decoding when reaching the end of the string to parse, or \c buf_end, whichever comes first. Return the
        /// parsed string, or throw \c error::malformed_mqtt_packet if input is malformed.
        ///
        /// \note           In MQTT 3.1.1 strings are encoded in UTF-8. They are preceded by two bytes in big
        ///                 endian order that encode string length.
        ///
        /// \param string_start     Start of buffer to parse. MUST point to first byte of two byte sequence
        ///                         encoding string length.
        /// \param buf_end          End of entire packet buffer, NOT end of string buffer (although that would
        ///                         work, too). Needed for range checks.
        /// \param parsed_string    A pointer to the parsed string (out parameter), will never be \c nullptr.
        /// \return                 The update InputIterator \c string_start, thus telling the caller where to
        ///                         continue decoding.
        /// \throws error::malformed_mqtt_packet        If decoding fails due to malformed input.
        /// \throws std::bad_alloc          If failing to allocate heap memory for \c parsed_string
        ///
        /// \pre        \c string_start  points to the first byte in a two byte sequence encoding string length,
        ///             followed by the bytes of the string itself.
        /// \pre        \c buf_end points immediately past the last byte in a buffer representing an
        ///             \c mqtt_packet's on the wire format.
        /// \post       The \c InputIterator returned points to the first unconsumed byte.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718016
        template <typename InputIterator>
        inline std::pair<InputIterator, const std::string> decode_utf8_string( InputIterator string_start,
                                                                               const InputIterator buf_end )
        {
            // We need at least two bytes for encoding string length
            if ( string_start + 2 > buf_end )
            {
                throw error::malformed_mqtt_packet( "Encoding an UTF-8 string needs at least two bytes" );
            }

            uint16_t string_length = -1;
            std::tie( string_start, string_length ) = decode_uint16( string_start, buf_end );
            // Do we have enough room for our string?
            if ( string_start + string_length > buf_end )
            {
                throw error::malformed_mqtt_packet( "Buffer truncated: cannot decode UTF-8 string" );
            }

            const char* parsed_string_ptr = reinterpret_cast<const char*>( &( *string_start ) );
            const std::string parsed_string{parsed_string_ptr, string_length};

            // Update buffer start iterator
            string_start += string_length;

            return {string_start, parsed_string};
        }

        /// \brief Interface for decoders capable of decoding a single type of MQTT packets.
        ///
        /// Defines and interface for decoders/decoders that take \c header_flags and a buffer containing an MQTT
        /// packet's on the wire representation and return a decoded \c mqtt_packet.
        ///
        /// Note that the concrete type of \c mqtt_packet to decode is already known when an implementation of this
        /// interface is called.
        ///
        /// Note further that a concrete \c packet_decoder_impl implementation handles exactly one concrete
        /// \c mqtt_packet type (CONNECT, CONNACK, ...).
        class packet_decoder_impl
        {
           public:
            /// \brief Parse the supplied buffer into an MQTT packet.
            ///
            /// Start decoding at \c buf_start. Parse until \c buf_end. Use \c header to create the parsed
            /// \c mqtt_packet and return it, transferring ownership to the caller. If decoding fails throw an
            /// \c error::malformed_mqtt_packet.
            ///
            /// \param header               Header of MQTT packet to parse. Contains the type of MQTT packet.
            /// \attention                  Implementations are asked to throw an assertion error if the the type
            ///                             of MQTT packet to parse contained in \c header does not match this
            ///                             implementation.
            /// \param buf_start            Start of buffer containing the serialized MQTT packet. MUST point to
            ///                             the start of the packet body, i.e. the variable header (if present) or
            ///                             the payload.
            /// \param buf_end              End of buffer containing the serialized MQTT packet.
            /// \return                     The parsed \c mqtt_packet, i.e. an instance of a concrete subclass of
            ///                             \c mqtt_packet. Note that the caller assumes ownership.
            /// \throws error::malformed_mqtt_packet    If encoding is malformed, e.g. remaining length has been
            ///                                         incorrectly encoded.
            ///
            /// \pre        \c buf_start points to the first byte after the fixed header in a buffer representing
            ///             an \c mqtt_packet's on the wire format.
            /// \pre        \c buf_end points immediately past the last byte in a buffer representing an
            ///             \c mqtt_packet's on the wire format.
            /// \pre        \c header is of the same MQTT Control Packet type as this \c packet_body_decoder
            ///             expects to decode.
            virtual std::shared_ptr<protocol::mqtt_packet> decode( const frame& frame ) const = 0;
        };  // packet_body_decoder

    }  /// namespace decoder
}  /// namespace io_wally
