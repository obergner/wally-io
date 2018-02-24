#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <tuple>
#include <vector>

#include "io_wally/codec/decoder.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_packet.hpp"

namespace io_wally
{
    namespace decoder
    {
        /// \brief \c packet_body_decoder implementation for PUBLISH packets.
        ///
        /// Interprets the supplied frame to contain a serialized PUBLISH packet. Decodes frame and returns
        /// decoded \c publish_packet.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718nullptr28
        class publish_packet_decoder_impl final : public packet_decoder_impl
        {
           public:
            /// \brief Decode supplied frame into a \c publish packet.
            ///
            /// \see io_wally::protocol::decoder::packet_body_decoder::parse
            ///
            virtual std::shared_ptr<protocol::mqtt_packet> decode( const frame& frame ) const
            {
                using namespace io_wally::protocol;

                assert( frame.type( ) == packet::Type::PUBLISH );

                // Check flags
                const auto flags = protocol::packet::header_flags{frame.type_and_flags};
                // [MQTT-3.3.1-2]: dup flag must not be set for QoS 0
                if ( flags.dup( ) && ( flags.qos( ) == protocol::packet::QoS::AT_MOST_ONCE ) )
                    throw error::malformed_mqtt_packet( "[MQTT-3.3.1-2] DUP flag set but QoS is 0 (at most once)" );

                // [MQTT-3.3.1-4]: QoS MUST NOT be RESERVED
                if ( flags.qos( ) == protocol::packet::QoS::RESERVED )
                    throw error::malformed_mqtt_packet( "[MQTT-3.3.1-4] QoS MUST NOT be 3 (RESERVED)" );

                auto new_buf_start = frame.begin;

                // Parse topic_name
                auto topic_name = std::string{};
                std::tie( new_buf_start, topic_name ) = decode_utf8_string( new_buf_start, frame.end );
                check_well_formed_topic_name( topic_name );

                // Parse variable header publish_header IFF QoS > 0
                auto packet_id = uint16_t{0};
                if ( ( flags.qos( ) == packet::QoS::AT_LEAST_ONCE ) || ( flags.qos( ) == packet::QoS::EXACTLY_ONCE ) )
                {
                    std::tie( new_buf_start, packet_id ) = decode_uint16( new_buf_start, frame.end );
                }

                // Parse application message
                auto application_message = std::vector<uint8_t>{new_buf_start, frame.end};

                return std::make_shared<protocol::publish>( frame.type_and_flags,
                                                            static_cast<const uint32_t>( frame.remaining_length( ) ),
                                                            topic_name, packet_id, application_message );
            }

           private:
            void check_well_formed_topic_name( const std::string& topic_name ) const
            {
                if ( topic_name.empty( ) )
                    throw error::malformed_mqtt_packet(
                        "[MQTT-4.7.3-1] Topic name MUST be at least one character long" );

                if ( topic_name.size( ) > 65535 )
                    throw error::malformed_mqtt_packet(
                        "[MQTT-4.7.3-3] Topic name MUST NOT be longer than 65535 characters" );

                std::size_t found = topic_name.find( '\0' );
                if ( found != std::string::npos )
                    throw error::malformed_mqtt_packet(
                        "[MQTT-4.7.3-2] Topic name MUST NOT include null character (U+0000)" );

                found = topic_name.find( '+' );
                if ( found != std::string::npos )
                    throw error::malformed_mqtt_packet(
                        "[MQTT-4.7.1-1] Topic name MUST NOT include wildcard character '+'" );

                found = topic_name.find( '#' );
                if ( found != std::string::npos )
                    throw error::malformed_mqtt_packet(
                        "[MQTT-4.7.1-1] Topic name MUST NOT include wildcard character '#'" );
            }
        };  // class publish_packet_decoder_impl
    }       // namespace decoder
}  // namespace io_wally
