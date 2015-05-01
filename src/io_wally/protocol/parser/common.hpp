#pragma once

#include <stdio.h>
#include <string.h>

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
            enum class ParseState : int
            {
                INCOMPLETE = 0,
                COMPLETE
            };

            /// \brief Stateful functor for parsing the 'remaining lenght' field in an MQTT fixed header.
            ///
            class remaining_length
            {
               public:
                remaining_length( )
                {
                    return;
                }

                /// \brief Calculate an MQTT packet's 'remaining length', i.e. its length in bytes minus fixed header
                ///        length.
                ///
                /// Take the next length byte 'next_byte'. Return 'parse_state' INCOMPLETE while calculation is not
                /// yet done. Once calculation has completed, return 'parse_state' COMPLETE and assign calculated
                /// 'remaining_lenght' to out parameter 'result'.
                ///
                /// Throw std::range_error if provided sequence of bytes does not encode a valid 'remaining length'.
                ///
                /// \param result Remaining length calculated by this functor.
                /// \param next_byte Next byte in sequence of bytes encoding an MQTT packet's remaining length.
                /// \return Current parse state, either INCOMPLETE or COMPLETE.
                /// \throws std::range_error If provided sequence of bytes does not encode a valid 'remaining length'.
                ParseState operator( )( uint32_t& result, const uint8_t next_byte )
                {
                    current_ += ( next_byte & ~MSB_MASK ) * multiplier_;
                    ParseState pst =
                        ( next_byte & MSB_MASK ) == MSB_MASK ? ParseState::INCOMPLETE : ParseState::COMPLETE;
                    if ( pst == ParseState::INCOMPLETE )
                        multiplier_ *= 128;

                    if ( multiplier_ > MAX_MULTIPLIER )
                        throw std::range_error( "supplied byte sequence does not encode a valid remaining length" );
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

            class header_parser
            {
               public:
                template <typename InputIterator>
                struct result
                {
                   public:
                    result( const ParseState parse_state )
                        : parse_state_( parse_state ), parsed_header_( boost::none ), consumed_until_( boost::none )
                    {
                        return;
                    }

                    result( const uint8_t type_and_flags,
                            const uint32_t remaining_length,
                            const InputIterator consumed_until )
                        : parse_state_( ParseState::COMPLETE ),
                          parsed_header_( packet::header( type_and_flags, remaining_length ) ),
                          consumed_until_( consumed_until )
                    {
                        return;
                    }

                    const ParseState parse_state( ) const
                    {
                        return parse_state_;
                    }

                    const bool is_parsing_complete( ) const
                    {
                        return parse_state_ == ParseState::COMPLETE;
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
                    const ParseState parse_state_;
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
                        return result<InputIterator>( ParseState::INCOMPLETE );

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
                remaining_length remaining_length_;
            };  // header_parser

            /// \brief Parse a 16 bit wide unsigned int in the supplied buffer.
            ///
            /// Start parsing at \c uint16_start, interpreting the supplied buffer as a 16 bit unsigned int in big
            /// endian byte order, as mandated by MQTT. Stop parsing when reaching the end of unsigned int to parse,
            /// or \c buf_end, whichever comes first. Return the parsed unsigned int, or throw \c std::range_error if
            /// input is malformed.
            ///
            /// \note           In MQTT 3.1.1 integers are encoded in big endian byte order.
            ///
            /// \param uint16_start     Start of buffer to parse.
            /// \param buf_end          End of entire packet buffer, NOT end of uint16_t buffer (although that would
            ///                         work, too). Needed for range checks.
            /// \param parsed_uint16    A pointer to the parsed integer (out parameter), will never be \c nullptr.
            /// \return                 The updated InputIterator \c uint16_start, thus telling the caller where to
            ///                         continue parsing.
            /// \throws std::range_error        If parsing fails due to malformed input.
            /// \throws std::bad_alloc          If failing to allocate heap memory for \c parsed_string
            ///
            /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718016
            template <typename InputIterator>
            inline InputIterator parse_uint16( InputIterator uint16_start,
                                               const InputIterator buf_end,
                                               uint16_t* const parsed_uint16 )
            {
                // We need at least two bytes for encoding a uint16_t
                if ( uint16_start + 2 > buf_end )
                {
                    throw std::range_error( "Encoding a 16 bit unsigned int needs at least two bytes" );
                }

                const uint8_t msb = *uint16_start++;
                const uint8_t lsb = *uint16_start++;
                *parsed_uint16 = ( msb << 8 ) + lsb;

                return uint16_start;
            }

            /// \brief Parse a UTF-8 string in the supplied buffer.
            ///
            /// Start parsing at \c string_start, interpreting the supplied buffer as an MQTT UTF-8 string. Stop
            /// parsing when reaching the end of the string to parse, or \c buf_end, whichever comes first. Return the
            /// parsed string, or throw \c std::range_error if input is malformed.
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
            ///                         continue parsing.
            /// \throws std::range_error        If parsing fails due to malformed input.
            /// \throws std::bad_alloc          If failing to allocate heap memory for \c parsed_string
            ///
            /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718016
            template <typename InputIterator>
            inline InputIterator parse_utf8_string( InputIterator string_start,
                                                    const InputIterator buf_end,
                                                    char** parsed_string )
            {
                // We need at least two bytes for encoding string length
                if ( string_start + 2 > buf_end )
                {
                    throw std::range_error( "Encoding an UTF-8 string needs at least two bytes" );
                }

                uint16_t string_length = -1;
                string_start = parse_uint16( string_start, buf_end, &string_length );
                // Do we have enough room for our string?
                if ( string_start + string_length > buf_end )
                {
                    throw std::range_error( "Buffer truncated: cannot decode UTF-8 string" );
                }

                *parsed_string = new char[string_length + 1];
                memcpy( *parsed_string, string_start, string_length + 1 );
                ( *parsed_string )[string_length] = '\0';

                // Update buffer start iterator
                string_start += string_length;

                return string_start;
            }

            /// \brief Interface for parsers capable of decoding a single type of MQTT packets.
            ///
            /// Defines and interface for parsers/decoders that take \c header_flags and a buffer containing an MQTT
            /// packet's on the wire representation and return a decoded \c mqtt_packet.
            ///
            /// Note that the concrete type of \c mqtt_packet to decode is already known when an implementation of this
            /// interface is called.
            ///
            /// Note further that a concrete \c packet_body_parser implementation handles exactly one concrete
            /// \c mqtt_packet type (CONNECT, CONNACK, ...).
            template <typename InputIterator>
            class packet_body_parser
            {
               public:
                /// \brief Parse the supplied buffer into an MQTT packet.
                ///
                /// From the supplied \c header_flags deduce the type of 'mqtt_packet' to parse. Start parsing at
                /// \c buf_start. Parse until \c buf_end. Return the parsed \c mqtt_packet, transferring ownership to
                /// the caller. If parsing fails throw a \c std::range_error.
                ///
                /// \param header               Header of MQTT packet to parse. Contains the type of MQTT packet.
                /// \param buf_start            Start of buffer containing the serialized MQTT packet. MUST point to
                ///                             the start of the packet body, i.e. the variable header (if present) or
                ///                             the payload.
                /// \param buf_end              End of buffer containing the serialized MQTT packet.
                /// \return                     The parsed \c mqtt_packet, i.e. an instance of a concrete subclass of
                ///                             \c mqtt_packe'. Note that the caller assumes ownership.
                /// \throws std::range_error    If encoding is malformed, e.g. remaining length has been incorrectly
                ///                             encoded.
                virtual std::unique_ptr<const mqtt_packet> parse( const packet::header& header,
                                                                  InputIterator buf_start,
                                                                  const InputIterator buf_end ) = 0;
            };

            /// \brief Parser for MQTT packets of arbitrary type.
            ///
            /// Takes a buffer containing an MQTT packet's on the wire representation, decodes its header and then
            /// delegates to an appropriate \c packet_body_parser implementation.
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
                /// Start parsing at \c buf_start, decoding this packet's \c fixed header. Continue parsing until
                /// \c buf_end. Return the parsed \c mqtt_packet, transferring ownership to the caller. If parsing fails
                /// throw a \c std::range_error.
                ///
                /// \param buf_start    Start of buffer containing the serialized MQTT packet. MUST point to the start
                ///                     of the entire packet.
                /// \param buf_end      End of buffer containing the serialized MQTT packet.
                /// \return             The parsed \c mqtt_packet, i.e. an instance of a concrete subclass of
                ///                     \c mqtt_packet. Note that the caller assumes ownership.
                /// \throws std::range_error    If encoding is malformed, e.g. remaining length has been incorrectly
                ///                             encoded.
                std::unique_ptr<const mqtt_packet> parse( const packet::header& header,
                                                          InputIterator buf_start,
                                                          const InputIterator buf_end )
                {
                    /// TODO: Implement
                    return std::unique_ptr<const mqtt_packet>( new mqtt_packet( header ) );
                }

                /// \brief Reset this \c mqtt_packet_parser's internal state, preparing it for reuse.
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
                remaining_length remaining_length_;
            };
        }  /// namespace parser
    }      /// namespace protocol
}  /// namespace io_wally
