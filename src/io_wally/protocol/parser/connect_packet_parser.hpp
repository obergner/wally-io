#pragma once

#include "io_wally/protocol/connect_packet.hpp"
#include "io_wally/protocol/parser/common.hpp"

namespace io_wally
{
    namespace protocol
    {
        namespace parser
        {
            /// \brief \c packet_body_parser implementation for CONNECT packets.
            ///
            /// Interprets the supplied buffer to contain a serialized CONNECT packet. Parses the buffer and returns
            /// decoded \c connect_packet.
            ///
            /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718nullptr28
            template <typename InputIterator>
            class connect_packet_parser : public packet_body_parser<InputIterator>
            {
               public:
                /// \brief Parse the supplied buffer into a \c connect packet.
                ///
                /// \see io_wally::protocol::parser::packet_body_parser::parse
                ///
                virtual std::unique_ptr<const mqtt_packet> parse( const packet::header& header,
                                                                  InputIterator buf_start,
                                                                  const InputIterator buf_end )
                {
                    // TODO:: consider removing this assert in release build
                    assert( header.type( ) == packet::Type::CONNECT );

                    InputIterator new_buf_start = buf_start;

                    // Parse variable header connect_header
                    char* protocol_name = nullptr;
                    new_buf_start = parse_utf8_string( new_buf_start, buf_end, &protocol_name );
                    if ( !protocol_name )
                        throw error::malformed_mqtt_packet( "CONNECT packet does not contain protocol name" );

                    const uint8_t protocol_level = (const uint8_t)*new_buf_start++;

                    const uint8_t connect_flags = (const uint8_t)*new_buf_start++;

                    uint16_t keep_alive_secs = -1;
                    new_buf_start = parse_uint16( new_buf_start, buf_end, &keep_alive_secs );

                    const struct connect_header connect_hdr(
                        protocol_name, protocol_level, connect_flags, keep_alive_secs );

                    // Parse payload connect_payload
                    char* client_id = nullptr;
                    new_buf_start = parse_utf8_string( new_buf_start, buf_end, &client_id );

                    char* last_will_topic = nullptr;
                    char* last_will_msg = nullptr;
                    if ( connect_hdr.contains_last_will( ) )
                    {
                        new_buf_start = parse_utf8_string( new_buf_start, buf_end, &last_will_topic );

                        new_buf_start = parse_utf8_string( new_buf_start, buf_end, &last_will_msg );
                    }

                    char* username = nullptr;
                    if ( connect_hdr.has_username( ) )
                    {
                        new_buf_start = parse_utf8_string( new_buf_start, buf_end, &username );
                    }

                    char* password = nullptr;
                    if ( connect_hdr.has_password( ) )
                    {
                        new_buf_start = parse_utf8_string( new_buf_start, buf_end, &password );
                    }

                    // TODO: consider removing this assert in release build
                    assert( new_buf_start == buf_end );

                    struct connect_payload connect_pyl( client_id, last_will_topic, last_will_msg, username, password );

                    std::unique_ptr<const mqtt_packet> result(
                        new connect( std::move( header ), std::move( connect_hdr ), std::move( connect_pyl ) ) );

                    return result;
                }
            };
        }
    }
}
