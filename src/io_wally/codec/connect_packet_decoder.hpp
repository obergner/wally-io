#pragma once

#include <cassert>
#include <cstdint>
#include <sstream>
#include <memory>

#include "io_wally/protocol/connect_packet.hpp"
#include "io_wally/codec/decoder.hpp"

namespace io_wally
{
    namespace decoder
    {
        /// \brief \c packet_body_decoder implementation for CONNECT packets.
        ///
        /// Interprets the supplied buffer to contain a serialized CONNECT packet. Decodes the buffer and returns
        /// decoded \c connect_packet.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718nullptr28
        template <typename InputIterator>
        class connect_packet_decoder : public packet_body_decoder<InputIterator>
        {
           public:
            /// \brief Decode the supplied buffer into a \c connect_packet.
            ///
            /// \see io_wally::protocol::decoder::packet_body_decoder::parse
            ///
            virtual std::unique_ptr<const protocol::mqtt_packet> decode( const protocol::packet::header& header,
                                                                         InputIterator buf_start,
                                                                         const InputIterator buf_end ) const
            {
                using namespace io_wally::protocol;

                // TODO:: consider removing this assert in release build
                assert( header.type( ) == packet::Type::CONNECT );

                // Check that size of supplied buffer corresponds to remaining length as advertised in header
                if ( ( buf_end - buf_start ) != header.remaining_length( ) )
                {
                    std::ostringstream message;
                    message << "Size of supplied buffer does not correspond to "
                            << "remaining length as advertised in header: [expected:" << header.remaining_length( )
                            << "|actual:" << ( buf_end - buf_start ) << "]";
                    throw error::malformed_mqtt_packet( message.str( ) );
                }

                InputIterator new_buf_start = buf_start;

                // Parse variable header connect_header
                char* protocol_name = nullptr;
                new_buf_start = decode_utf8_string( new_buf_start, buf_end, &protocol_name );
                if ( !protocol_name )
                    throw error::malformed_mqtt_packet( "CONNECT packet does not contain protocol name" );

                const uint8_t protocol_level = (const uint8_t)*new_buf_start++;

                const uint8_t connect_flags = (const uint8_t)*new_buf_start++;
                const protocol::connect_flags cf{connect_flags};

                uint16_t keep_alive_secs = -1;
                new_buf_start = decode_uint16( new_buf_start, buf_end, &keep_alive_secs );

                // Parse payload connect_payload
                char* client_id = nullptr;
                new_buf_start = decode_utf8_string( new_buf_start, buf_end, &client_id );

                char* last_will_topic = nullptr;
                char* last_will_msg = nullptr;
                if ( cf.contains_last_will( ) )
                {
                    new_buf_start = decode_utf8_string( new_buf_start, buf_end, &last_will_topic );

                    new_buf_start = decode_utf8_string( new_buf_start, buf_end, &last_will_msg );
                }

                char* username = nullptr;
                if ( cf.has_username( ) )
                {
                    new_buf_start = decode_utf8_string( new_buf_start, buf_end, &username );
                }

                char* password = nullptr;
                if ( cf.has_password( ) )
                {
                    new_buf_start = decode_utf8_string( new_buf_start, buf_end, &password );
                }

                if ( new_buf_start != buf_end )
                    throw error::malformed_mqtt_packet(
                        "Combined size of fields in buffers does not add up to advertised remaining length" );

                return std::make_unique<const protocol::connect>( header,
                                                                  protocol_name,
                                                                  protocol_level,
                                                                  connect_flags,
                                                                  keep_alive_secs,
                                                                  client_id,
                                                                  last_will_topic,
                                                                  last_will_msg,
                                                                  username,
                                                                  password );
            }
        };  // class connect_packet_decoder

    }  // namespace decoder
}  // namespace io_wally
