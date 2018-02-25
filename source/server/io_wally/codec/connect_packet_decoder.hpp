#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <tuple>

#include "io_wally/codec/decoder.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/connect_packet.hpp"

namespace io_wally
{
    namespace decoder
    {
        class connect_packet_decoder_impl final : public packet_decoder_impl
        {
           public:
            /// \brief Decode the supplied buffer into a \c connect_packet.
            ///
            /// \see io_wally::protocol::decoder::packet_body_decoder::parse
            ///
            virtual std::shared_ptr<protocol::mqtt_packet> decode( const frame& frame ) const
            {
                using namespace io_wally::protocol;

                assert( frame.type( ) == packet::Type::CONNECT );

                if ( ( frame.type_and_flags & 0x0F ) != 0x00 )
                {
                    throw error::malformed_mqtt_packet{
                        "[MQTT-2.2.2-2] CONNECT packet contains invalid fixed header flags"};
                }

                auto new_buf_start = frame.begin;

                // Parse variable header connect_header
                auto protocol_name = std::string{};
                std::tie( new_buf_start, protocol_name ) = decode_utf8_string( new_buf_start, frame.end );
                if ( protocol_name.empty( ) )
                {
                    throw error::malformed_mqtt_packet{"CONNECT packet does not contain protocol name"};
                }

                const auto protocol_level = *new_buf_start++;

                const auto connect_flags = *new_buf_start++;
                const auto cf = protocol::connect_flags{connect_flags};

                auto keep_alive_secs = uint16_t{0};
                std::tie( new_buf_start, keep_alive_secs ) = decode_uint16( new_buf_start, frame.end );

                // Parse payload connect_payload
                auto client_id = std::string{};
                std::tie( new_buf_start, client_id ) = decode_utf8_string( new_buf_start, frame.end );

                auto last_will_topic = std::string{};
                auto last_will_msg = std::vector<uint8_t>{};
                if ( cf.contains_last_will( ) )
                {
                    std::tie( new_buf_start, last_will_topic ) = decode_utf8_string( new_buf_start, frame.end );
                    auto last_will_msg_len = uint16_t{0};
                    std::tie( new_buf_start, last_will_msg_len ) = decode_uint16( new_buf_start, frame.end );
                    if ( last_will_msg_len > std::distance( new_buf_start, frame.end ) )
                    {
                        throw error::malformed_mqtt_packet{
                            "Length of last will message in CONNECT packet exceeds advertised remaining length"};
                    }
                    last_will_msg = std::vector<uint8_t>{new_buf_start, new_buf_start += last_will_msg_len};
                }

                auto username = std::string{};
                if ( cf.has_username( ) )
                {
                    std::tie( new_buf_start, username ) = decode_utf8_string( new_buf_start, frame.end );
                }

                auto password = std::string{};
                if ( cf.has_password( ) )
                {
                    std::tie( new_buf_start, password ) = decode_utf8_string( new_buf_start, frame.end );
                }

                if ( new_buf_start != frame.end )
                {
                    throw error::malformed_mqtt_packet{
                        "Combined size of fields in buffers does not add up to advertised remaining length"};
                }

                return std::make_shared<protocol::connect>(
                    static_cast<const uint32_t>( frame.remaining_length( ) ), protocol_name.c_str( ), protocol_level,
                    connect_flags, keep_alive_secs, client_id.c_str( ), last_will_topic.c_str( ), last_will_msg,
                    username.c_str( ), password.c_str( ) );
            }
        };  // class connect_packet_decoder_impl

    }  // namespace decoder
}  // namespace io_wally
