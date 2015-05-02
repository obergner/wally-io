#pragma once

#include "io_wally/protocol/parser/connect_packet_parser.hpp"

using namespace io_wally::protocol;

namespace io_wally
{
    namespace protocol
    {
        namespace parser
        {
            /// \brief Parser for MQTT packets of arbitrary but known type.
            ///
            /// Takes an MQTT \c header and a buffer containing an MQTT packet's on the wire representation. Deduces
            /// from the supplied \c header the type of \c mqtt_packet to parse and delegates to an appropriate
            /// \c packet_body_parser implementation.
            template <typename InputIterator>
            class mqtt_packet_parser
            {
               public:
                /// Methods

                /// \brief Parse the supplied buffer into an MQTT packet.
                ///
                /// From the supplied \c header determine the concrete type of \c mqtt_packet to parse. Find an
                /// appropriate implementation of \c packet_body_parser and delegate parsing to it. Return the parsed
                /// \c mqtt_packet, transferring ownership to the caller. If parsing fails throw an
                /// \c error::malformed_mqtt_packet.
                ///
                /// \param header       Parsed \c header of MQTT control packet to decode.
                /// \param buf_start    Start of buffer containing the serialized MQTT packet. MUST point immediately
                ///                     past the fixed header.
                /// \param buf_end      End of buffer containing the serialized MQTT packet.
                /// \return             The parsed \c mqtt_packet, i.e. an instance of a concrete subclass of
                ///                     \c mqtt_packet. Note that the caller assumes ownership.
                /// \throws error::malformed_mqtt_packet    If encoding is malformed, e.g. remaining length has been
                ///                                         incorrectly encoded.
                ///
                /// \pre        \c buf_start points to the first byte after the fixed header in a buffer representing
                ///             an \c mqtt_packet's on the wire format.
                /// \pre        \c buf_end points immediately past the last byte in a buffer representing an
                ///             \c mqtt_packet's on the wire format.
                std::unique_ptr<const mqtt_packet> parse( const packet::header& header,
                                                          InputIterator buf_start,
                                                          const InputIterator buf_end )
                {
                    return body_parser_for( header ).parse( header, buf_start, buf_end );
                }

               private:
                /// Methods
                const packet_body_parser<InputIterator>& body_parser_for( const packet::header& header )
                {
                    switch ( header.type( ) )
                    {
                        case packet::Type::CONNECT:
                            return connect_packet_parser_;
                        default:
                            throw std::invalid_argument( "Unsupported MQTT control packet type" );
                    }

                    assert( false );
                }

                /// Fields
                const connect_packet_parser<InputIterator> connect_packet_parser_ =
                    connect_packet_parser<InputIterator>( );
            };

        }  /// namespace parser
    }      /// namespace protocol
}  /// namespace io_wally
