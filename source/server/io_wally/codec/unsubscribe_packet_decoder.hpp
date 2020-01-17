#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <tuple>
#include <vector>

#include "io_wally/codec/decoder.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/unsubscribe_packet.hpp"

namespace io_wally
{
    namespace decoder
    {
        /// \brief \c packet_body_decoder implementation for UNSUBSCRIBE packets.
        ///
        /// Interprets supplied \c frame to contain a serialized UNSUBSCRIBE packet. Decodes \c frame and returns
        /// decoded \c unsubscribe_packet.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718nullptr28
        class unsubscribe_packet_decoder_impl final : public packet_decoder_impl
        {
           public:
            /// \brief Decode supplied \c frame into a \c unsubscribe packet.
            ///
            /// \see io_wally::protocol::decoder::packet_body_decoder::parse
            ///
            virtual std::shared_ptr<protocol::mqtt_packet> decode( const frame& frame ) const
            {
                using namespace io_wally::protocol;

                assert( frame.type( ) == packet::Type::UNSUBSCRIBE );

                // Check that header flags comply with MQTT spec
                // See: [MQTT-3.10.1-1]
                if ( ( frame.type_and_flags & 0x0F ) != 0x02 )
                    throw error::malformed_mqtt_packet{"[MQTT-3.10.1-1] Illegal header flags in UNSUBSCRIBE packet"};

                auto new_buf_start = frame.begin;

                // Parse variable header unsubscribe_header
                auto packet_id = uint16_t{0};
                std::tie( new_buf_start, packet_id ) = decode_uint16( new_buf_start, frame.end );

                // Parse payload unsubscribe_payload
                auto topic_filters = std::vector<std::string>{};
                while ( new_buf_start != frame.end )
                {
                    auto topic_filter = std::string{};
                    std::tie( new_buf_start, topic_filter ) = decode_utf8_string( new_buf_start, frame.end );

                    topic_filters.emplace_back( topic_filter );
                }
                if ( topic_filters.empty( ) )
                    throw error::malformed_mqtt_packet{
                        "[MQTT-3.10.3-2] A UNSUBSCRIBE packet MUST contain at least one subscription (topic filter)"};

                return std::make_shared<protocol::unsubscribe>( static_cast<uint32_t>( frame.remaining_length( ) ),
                                                                packet_id, topic_filters );
            }
        };  // class unsubscribe_packet_decoder_impl
    }       // namespace decoder
}  // namespace io_wally
