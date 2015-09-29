#pragma once

#include <stdio.h>
#include <string.h>

#include <stdexcept>
#include <memory>
#include <tuple>

#include <boost/optional.hpp>

#include "io_wally/protocol/common.hpp"

using namespace io_wally::protocol;

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

        /// \brief Namespace for grouping all exceptions that may be thrown when decoding an \c mqtt_packet's on
        ///        the wire representation.
        namespace error
        {
            /// \brief Signals an incorrectly encoded MQTT control packet.
            ///
            /// Examples of when a \c malformed_mqtt_packet will be thrown include, but are not limited to:
            ///
            ///  - Incorrectly encoded remaining length
            ///  - Packet is actually shorter than advertised in its remaining length field
            ///  - Username field was flagged as being present in a \c connect packet's variable header, yet is
            ///    actually missing in the payload
            ///
            class malformed_mqtt_packet : public std::runtime_error
            {
               public:
                /// Create a new \c malformed_mqtt_packet instance, using the supplied reason.
                ///
                /// \param what      Reason
                malformed_mqtt_packet( const std::string& what ) : runtime_error( what )
                {
                    return;
                }
            };
        }

        /// \brief Denotes whether decoding a datum has been completed (sufficient input) or not (furhter input
        ///        needed).
        ///
        enum class ParseState : int
        {
            /// \brief decoding is not yet complete. More input is needed.
            INCOMPLETE = 0,

            /// \brief decoding is complete. The parse result is available.
            COMPLETE
        };

        /// \brief Stateful functor for decoding the \c remaining lenght field in an MQTT fixed header.
        ///
        class remaining_length
        {
           public:
            /// \brief Calculate an MQTT packet's \c remaining length, i.e. its length in bytes minus fixed header
            ///        length.
            ///
            /// Take the next length byte \c next_byte. Return \c ParseState INCOMPLETE while calculation is not
            /// yet done. Once calculation has completed, return \c ParseState COMPLETE and assign calculated
            /// \c remaining_lenght to out parameter \c result.
            ///
            /// Throw error::malformed_mqtt_packet if provided sequence of bytes does not encode a valid \c
            /// remaining length.
            ///
            /// \param result       Remaining length calculated by this functor.
            /// \param next_byte    Next byte in sequence of bytes encoding an MQTT packet's remaining length.
            /// \return             Current \c ParseState, either INCOMPLETE or COMPLETE.
            /// \throws error::malformed_mqtt_packet If provided sequence of bytes does not encode a valid
            ///                                      \c remaining length.
            ///
            /// \pre        Either this functor is called for the first time after creation/calling reset(), or
            ///             the previous call has returned \c ParseState::INCOMPLETE
            ParseState operator( )( uint32_t& result, const uint8_t next_byte )
            {
                current_ += ( next_byte & ~MSB_MASK ) * multiplier_;
                ParseState pst = ( next_byte & MSB_MASK ) == MSB_MASK ? ParseState::INCOMPLETE : ParseState::COMPLETE;
                if ( pst == ParseState::INCOMPLETE )
                    multiplier_ *= 128;

                if ( multiplier_ > MAX_MULTIPLIER )
                    throw error::malformed_mqtt_packet(
                        "supplied byte sequence does not encode a valid remaining length" );
                if ( pst == ParseState::COMPLETE )
                    result = current_;

                return pst;
            }

            /// \brief Reset this functor's internal state so that it may be reused.
            void reset( )
            {
                current_ = 0;
                multiplier_ = 1;
            }

           private:
            const uint8_t MSB_MASK = 0x80;
            const uint32_t MAX_MULTIPLIER = 128 * 128 * 128;
            uint32_t current_ = 0;
            uint32_t multiplier_ = 1;
        };

        /// \brief Stateful decoder for MQTT \c headers.
        ///
        class header_decoder
        {
           public:
            /// \brief Result of decoding an MQTT \c header.
            ///
            /// Combines current \c ParseState - one of COMPLETE and INCOMPLETE - as well as optionally the parsed
            /// \c header and the updated \c InputIterator. Parsed \c header and the updated \c InputIterator are
            /// defined if and only if \c ParseState is COMPLETE.
            template <typename InputIterator>
            struct result
            {
               public:
                /// \brief Create a result instance in \c ParseState \c ParseState::INCOMPLETE.
                result( )
                    : parse_state_{ParseState::INCOMPLETE}, parsed_header_{boost::none}, consumed_until_{boost::none}
                {
                    return;
                }

                /// \brief Create a result instance in \c ParseState \c ParseState::COMPLETE.
                ///
                /// \param type_and_flags   Byte encoding packet type and packet flags
                /// \param remaining_length Remaining length (excluding header) of packet
                /// \param consumed_until   \c InputIterator that points past the last consumed byte
                result( const uint8_t type_and_flags,
                        const uint32_t remaining_length,
                        const InputIterator consumed_until )
                    : parse_state_{ParseState::COMPLETE},
                      parsed_header_{packet::header( type_and_flags, remaining_length )},
                      consumed_until_{consumed_until}
                {
                    return;
                }

                /// \brief Return \c ParseState, either \c ParseState::COMPLETE or \c ParseState::INCOMPLETE.
                ParseState parse_state( ) const
                {
                    return parse_state_;
                }

                /// \brief Return whether decoding has completed or not.
                bool is_parsing_complete( ) const
                {
                    return parse_state_ == ParseState::COMPLETE;
                }

                /// \brief Return the parsed \c header.
                ///
                /// \attention This method MUST NOT BE CALLED if \c parse_state() is \c ParseState::INCOMPLETE.
                const packet::header parsed_header( ) const
                {
                    return parsed_header_.get( );
                }

                /// \brief Returned an \c InputIterator pointing past the last consumed byte.
                const InputIterator consumed_until( ) const
                {
                    return consumed_until_.get( );
                }

               private:
                const ParseState parse_state_;
                const boost::optional<packet::header> parsed_header_;
                const boost::optional<InputIterator> consumed_until_;
            };

            /// \brief Create a default \c header_decoder.
            header_decoder( ) : type_and_flags_{-1}, remaining_length_{}
            {
                return;
            }

            /// \brief Parse the supplied buffer into an MQTT \c header, potentially in multiple consecutive calls.
            ///
            /// Start decoding the supplied buffer at \c buf_start, continuing until a complete \c header has been
            /// parsed, or until \c buf_end, whichever comes first.
            ///
            /// Return a \c result with \c ParseState INCOMPLETE if the supplied buffer holds only a \c header
            /// part. In that case the \c result returned will contain an \c InputIterator pointing past the last
            /// consumed byte. The caller is expected to supply the next chunk in a subsequent call to this method,
            /// with \c buf_start set to the \c InputIterator previously returned.
            ///
            /// If an entire \c header could be parsed, return a \c result with \c ParseState COMPLETE and the
            /// parsed \c header. Again, the \c result returned contains an \c InputIterator pointing past the last
            /// consumed byte.
            ///
            /// If the supplied buffer contains malformed input, throw an error::malformed_mqtt_packet.
            ///
            /// \param buf_start    Start of supplied buffer
            /// \param buf_end      End of supplied buffer
            /// \return             The parse \c result, either COMPLETE or INCOMPLETE. Always contains the
            ///                     updated \c InputIterator signalling the caller where to continue decoding. If
            ///                     decoding has completed, also contains the parsed \c header
            /// \throws error::malformed_mqtt_packet    If the supplied buffer does not contain a correctly encoded
            ///                                         \c header.
            ///
            /// \pre        \c buf_start initially points to the first byte in a buffer representing an
            ///             \c mqtt_packet's on the wire format (first call), or to the first unconsumed byte
            ///             (subsequent calls).
            /// \pre        \c buf_end points immediately past the last byte in a buffer representing an
            ///             \c mqtt_packet's on the wire format.
            /// \post       The \c InputIterator returned in \c result<InputIterator> points to the first
            ///             unconsumed byte.
            /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718020
            ///
            template <typename InputIterator>
            const result<InputIterator> decode( InputIterator buf_start, const InputIterator buf_end )
            {
                uint32_t rem_len = -1;
                ParseState rem_len_pst = ParseState::INCOMPLETE;
                while ( ( buf_start != buf_end ) && ( rem_len_pst == ParseState::INCOMPLETE ) )
                {
                    if ( type_and_flags_ == -1 )
                    {
                        type_and_flags_ = *buf_start++;
                    }
                    else
                    {
                        rem_len_pst = remaining_length_( rem_len, *buf_start++ );
                    }
                }

                if ( rem_len_pst == ParseState::INCOMPLETE )
                    return result<InputIterator>( );

                return result<InputIterator>( type_and_flags_, rem_len, buf_start );
            }

            /// \brief Reset this \c header_decoder's internal state, preparing it for reuse.
            ///
            void reset( )
            {
                type_and_flags_ = -1;
                remaining_length_.reset( );
            }

           private:
            /// Fields
            int16_t type_and_flags_;
            remaining_length remaining_length_;
        };  // header_decoder

        /// \brief Parse a 16 bit wide unsigned int in the supplied buffer.
        ///
        /// Start decoding at \c uint16_start, interpreting the supplied buffer as a 16 bit unsigned int in big
        /// endian byte order, as mandated by MQTT. Stop decoding when reaching the end of unsigned int to parse,
        /// or \c buf_end, whichever comes first. Return the parsed unsigned int, or throw
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
        /// \throws std::bad_alloc          If failing to allocate heap memory for \c parsed_string
        ///
        /// \pre        \c uint16_start initially points to the first byte of a two byte sequence in big endian.
        /// \pre        \c buf_end points immediately past the last byte in a buffer representing an
        ///             \c mqtt_packet's on the wire format.
        /// \post       The \c InputIterator returned points to the first unconsumed byte.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718016
        template <typename InputIterator>
        inline InputIterator decode_uint16( InputIterator uint16_start,
                                            const InputIterator buf_end,
                                            uint16_t* const parsed_uint16 )
        {
            // We need at least two bytes for encoding a uint16_t
            if ( uint16_start + 2 > buf_end )
            {
                throw error::malformed_mqtt_packet( "Encoding a 16 bit unsigned int needs at least two bytes" );
            }

            const uint8_t msb = *uint16_start++;
            const uint8_t lsb = *uint16_start++;
            *parsed_uint16 = ( msb << 8 ) + lsb;

            return uint16_start;
        }

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
        inline InputIterator decode_utf8_string( InputIterator string_start,
                                                 const InputIterator buf_end,
                                                 char** parsed_string )
        {
            // We need at least two bytes for encoding string length
            if ( string_start + 2 > buf_end )
            {
                throw error::malformed_mqtt_packet( "Encoding an UTF-8 string needs at least two bytes" );
            }

            uint16_t string_length = -1;
            string_start = decode_uint16( string_start, buf_end, &string_length );
            // Do we have enough room for our string?
            if ( string_start + string_length > buf_end )
            {
                throw error::malformed_mqtt_packet( "Buffer truncated: cannot decode UTF-8 string" );
            }

            *parsed_string = new char[string_length + 1];
            memcpy( *parsed_string, &( *string_start ), string_length + 1 );
            ( *parsed_string )[string_length] = '\0';

            // Update buffer start iterator
            string_start += string_length;

            return string_start;
        }

        /// \brief Interface for decoders capable of decoding a single type of MQTT packets.
        ///
        /// Defines and interface for decoders/decoders that take \c header_flags and a buffer containing an MQTT
        /// packet's on the wire representation and return a decoded \c mqtt_packet.
        ///
        /// Note that the concrete type of \c mqtt_packet to decode is already known when an implementation of this
        /// interface is called.
        ///
        /// Note further that a concrete \c packet_body_decoder implementation handles exactly one concrete
        /// \c mqtt_packet type (CONNECT, CONNACK, ...).
        template <typename InputIterator>
        class packet_body_decoder
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
            virtual std::unique_ptr<const mqtt_packet> decode( const packet::header& header,
                                                               InputIterator buf_start,
                                                               const InputIterator buf_end ) const = 0;
        };  // packet_body_decoder

    }  /// namespace decoder
}  /// namespace io_wally
