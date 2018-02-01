#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <tuple>
#include <vector>

#include "io_wally/codec/decoder.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/protocol/subscription.hpp"

namespace io_wally
{
    namespace decoder
    {
        /// \brief \c packet_body_decoder implementation for SUBSCRIBE packets.
        ///
        /// Interprets supplied \c frame to contain a serialized SUBSCRIBE packet. Decodes \c frame and returns
        /// decoded \c subscribe_packet.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718nullptr28
        class subscribe_packet_decoder_impl final : public packet_decoder_impl
        {
           public:
            /// \brief Decode supplied \c frame into a \c subscribe packet.
            ///
            /// \see io_wally::protocol::decoder::packet_body_decoder::parse
            ///
            virtual std::shared_ptr<protocol::mqtt_packet> decode( const frame& frame ) const
            {
                using namespace io_wally::protocol;

                assert( frame.type( ) == packet::Type::SUBSCRIBE );

                // Check that header flags comply with MQTT spec
                // See: [MQTT-3.8.1-1]
                if ( ( frame.type_and_flags & 0x0F ) != 0x02 )
                    throw error::malformed_mqtt_packet( "[MQTT-3.8.1-1] Illegal header flags in SUBSCRIBE packet" );

                auto new_buf_start = frame.begin;

                // Parse variable header subscribe_header
                auto packet_id = uint16_t{0};
                std::tie( new_buf_start, packet_id ) = decode_uint16( new_buf_start, frame.end );

                // Parse payload subscribe_payload
                auto subscriptions = std::vector<subscription>{};
                while ( new_buf_start != frame.end )
                {
                    // char* topic_filter = nullptr;
                    auto topic_filter = std::string{};
                    std::tie( new_buf_start, topic_filter ) = decode_utf8_string( new_buf_start, frame.end );

                    auto maximum_qos = packet::QoS{};
                    new_buf_start = decode_qos( new_buf_start, frame.end, &maximum_qos );

                    subscriptions.emplace_back( topic_filter, maximum_qos );
                }
                if ( subscriptions.empty( ) )
                    throw error::malformed_mqtt_packet(
                        "[MQTT-3.8.3-3] A SUBSCRIBE packet MUST contain at least one subscription (topic filter/QoS "
                        "pair)" );

                return std::make_shared<protocol::subscribe>(
                    protocol::packet::header{frame.type_and_flags,
                                             static_cast<const uint32_t>( frame.remaining_length( ) )},
                    packet_id, subscriptions );
            }
        };  // class subscribe_packet_decoder_impl
    }       // namespace decoder
}  // namespace io_wally
