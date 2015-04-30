#pragma once

#include <tuple>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include "io_wally/protocol/common.hpp"

using namespace io_wally::protocol;

namespace io_wally
{
    namespace protocol
    {
        namespace parser
        {
            enum parse_state : int
            {
                INCOMPLETE = 0,
                COMPLETE
            };

            class header_parser
            {
               public:
                template <typename InputIterator>
                struct result
                {
                   public:
                    result( const enum parse_state parse_state )
                        : parse_state_( parse_state ), parsed_header_( boost::none ), consumed_until_( boost::none )
                    {
                        return;
                    }

                    result( const uint8_t type_and_flags,
                            const uint32_t remaining_length,
                            const InputIterator consumed_until )
                        : parse_state_( COMPLETE ),
                          parsed_header_( packet::header( type_and_flags, remaining_length ) ),
                          consumed_until_( consumed_until )
                    {
                        return;
                    }

                    const enum parse_state parse_state( ) const
                    {
                        return parse_state_;
                    }

                    const bool is_parsing_complete( ) const
                    {
                        return parse_state_ == COMPLETE;
                    }

                    const packet::header parsed_header( ) const
                    {
                        return parsed_header_.get( );
                    }

                    const InputIterator consumed_until( ) const
                    {
                        return consumed_until_.get( );
                    }

                   private:
                    const enum parse_state parse_state_;
                    const boost::optional<packet::header> parsed_header_;
                    const boost::optional<InputIterator> consumed_until_;
                };
                header_parser( ) : type_and_flags_( -1 ), remaining_length_( )
                {
                    return;
                }

                template <typename InputIterator>
                const result<InputIterator> parse( InputIterator buf_start, const InputIterator buf_end )
                {
                    uint32_t rem_len = -1;
                    packet::remaining_length::parse_state rem_len_pst = packet::remaining_length::INCOMPLETE;
                    while ( ( buf_start != buf_end ) && ( rem_len_pst == packet::remaining_length::INCOMPLETE ) )
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

                    if ( rem_len_pst == packet::remaining_length::INCOMPLETE )
                        return result<InputIterator>( INCOMPLETE );

                    return result<InputIterator>( type_and_flags_, rem_len, buf_start );
                }

                void reset( )
                {
                    type_and_flags_ = -1;
                    remaining_length_.reset( );
                }

               private:
                /// Fields
                int16_t type_and_flags_;
                packet::remaining_length remaining_length_;
            };

            /// \brief Parse an UTF-8 string in the supplied buffer.
            ///
            /// Start parsing at 'string_start', interpreting the supplied buffer as an MQTT UTF-8 string. Stop
            /// parsing when reaching the end of the string to parse, or 'buf_end', whichever comes first. Return the
            /// parsed string, or throw std::range_error if input is malformed.
            ///
            /// \note           In MQTT 3.1.1 strings are encoded in UTF-8. They are preceded by two bytes in big
            ///                 endian order that encode string length.
            ///
            /// \param string_start     Start of buffer to parse. MUST point to first byte of two byte sequence
            ///                         encoding string length.
            /// \param buf_end          End of entire packet buffer, NOT end of string buffer (although that would
            ///                         work, too). Needed for range checks.
            /// \return                 The parsed string.
            /// \throws std::ranger_error       If parsing fails due to malformed input.
            ///
            /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718016
            template <typename InputIterator>
            inline const std::string parse_utf8_string( InputIterator string_start, const InputIterator buf_end )
            {
                // We need at least two bytes for encoding string length
                if ( string_start + 2 > buf_end )
                {
                    throw std::range_error( "Encoding an UTF-8 string needs at least two bytes" );
                }

                const uint8_t msb = *string_start++;
                const uint8_t lsb = *string_start++;
                const uint16_t string_length = ( msb << 8 ) + lsb;
                // Do we have enough room for our string?
                if ( string_start + string_length > buf_end )
                {
                    throw std::range_error( "Buffer truncated: cannot decode UTF-8 string" );
                }

                return std::string( string_start, string_length );
            }

            /// \brief Interface for parsers capable of decoding a single type of MQTT packets.
            ///
            /// Defines and interface for parsers/decoders that take header_flags and a buffer containing an MQTT
            /// packet's on the wire representation and return a decoded mqtt_packet.
            ///
            /// Note that the concrete type of mqtt_packet to decode is already known when an implementation of this
            /// interface is called.
            ///
            /// Note further that a concrete packet_body_parser implementation handles exactly one concrete
            /// mqtt_packet type (CONNECT, CONNACK, ...).
            template <typename InputIterator>
            class packet_body_parser
            {
               public:
                virtual ~packet_body_parser( )
                {
                    return;
                }

                /// \brief Parse the supplied buffer into an MQTT packet.
                ///
                /// From the supplied 'header_flags' deduce the type of 'mqtt_packet' to parse. Start parsing at
                /// 'buf_start'. Parse until 'buf_end'. Return the parsed 'mqtt_packet', transferring ownership to the
                /// caller. If parsing fails throw a std::range_error.
                ///
                /// \param header_flags         Header flags of MQTT packet to parse. Contains the type of MQTT packet.
                /// \param buf_start            Start of buffer containing the serialized MQTT packet. MUST point to
                ///                             the start of the packet body, i.e. the variable header (if present) or
                ///                             the payload.
                /// \param buf_end              End of buffer containing the serialized MQTT packet.
                /// \return                     The parsed 'mqtt_packet', i.e. an instance of a concrete subclass of
                ///                             'mqtt_packet'. Note that the caller assumes ownership.
                /// \throws std::range_error    If encoding is malformed, e.g. remaining length has been incorrectly
                ///                             encoded.
                virtual const std::unique_ptr<mqtt_packet> parse( const packet::header_flags& header_flags,
                                                                  InputIterator buf_start,
                                                                  const InputIterator buf_end ) = 0;
            };

            /// \brief Parser for MQTT packets of arbitrary type.
            ///
            /// Takes a buffer containing an MQTT packet's on the wire representation, decodes its header and then
            /// delegates to an appropriate 'packet_body_parser' implementation.
            template <typename InputIterator>
            class mqtt_packet_parser
            {
               public:
                /// Methods
                mqtt_packet_parser( ) : remaining_length_( )
                {
                    return;
                }

                /// \brief Parse the supplied buffer into an MQTT packet.
                ///
                /// Start parsing at 'buf_start', decoding this packet's 'fixed header'. Continue parsing until
                /// 'buf_end'. Return the parsed 'mqtt_packet', transferring ownership to the caller. If parsing fails
                /// throw a std::range_error.
                ///
                /// \param buf_start    Start of buffer containing the serialized MQTT packet. MUST point to the start
                ///                     of the entire packet.
                /// \param buf_end      End of buffer containing the serialized MQTT packet.
                /// \return             The parsed 'mqtt_packet', i.e. an instance of a concrete subclass of
                ///                     'mqtt_packet'. Note that the caller assumes ownership.
                /// \throws std::range_error    If encoding is malformed, e.g. remaining length has been incorrectly
                ///                             encoded.
                std::unique_ptr<const mqtt_packet> parse( const packet::header& header,
                                                          InputIterator buf_start,
                                                          const InputIterator buf_end )
                {
                    /// TODO: Implement
                    return std::unique_ptr<const mqtt_packet>( new mqtt_packet( header ) );
                }

                /// \brief Reset this 'mqtt_packet_parser's' internal state, preparing it for reuse.
                void reset( )
                {
                    remaining_length_.reset( );
                }

               private:
                /// Methods
                /*
                const packet_body_parser& body_parser_for( const packet::header& header )
                {
                }
                */

                /// Fields
                packet::remaining_length remaining_length_;
            };
        }  /// namespace parser
    }      /// namespace protocol
}  /// namespace io_wally
