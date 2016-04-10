#pragma once

#include <cassert>
#include <cstdint>
#include <sstream>
#include <memory>
#include <vector>
#include <tuple>

#include "io_wally/protocol/subscription.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/codec/decoder.hpp"

namespace io_wally
{
    namespace decoder
    {
        /// \brief \c packet_body_decoder implementation for SUBSCRIBE packets.
        ///
        /// Interprets the supplied buffer to contain a serialized SUBSCRIBE packet. Decodes the buffer and returns
        /// decoded \c subscribe_packet.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718nullptr28
        template <typename InputIterator>
        class subscribe_packet_decoder final : public packet_body_decoder<InputIterator>
        {
           public:
            /// \brief Decode the supplied buffer into a \c subscribe packet.
            ///
            /// \see io_wally::protocol::decoder::packet_body_decoder::parse
            ///
            virtual std::shared_ptr<protocol::mqtt_packet> decode( const protocol::packet::header& header,
                                                                   InputIterator buf_start,
                                                                   const InputIterator buf_end ) const
            {
                using namespace io_wally::protocol;

                assert( header.type( ) == packet::Type::SUBSCRIBE );

                // Check that header flags comply with MQTT spec
                // See: [MQTT-3.8.1-1]
                if ( ( header.type_and_flags( ) & 0x0F ) != 0x02 )
                    throw error::malformed_mqtt_packet( "[MQTT-3.8.1-1] Illegal header flags in SUBSCRIBE packet" );

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

                // Parse variable header subscribe_header
                uint16_t packet_identifier = -1;
                std::tie( new_buf_start, packet_identifier ) = decode_uint16( new_buf_start, buf_end );

                // Parse payload subscribe_payload
                std::vector<subscription> subscriptions{};
                while ( new_buf_start != buf_end )
                {
                    // char* topic_filter = nullptr;
                    std::string topic_filter;
                    std::tie( new_buf_start, topic_filter ) = decode_utf8_string( new_buf_start, buf_end );

                    packet::QoS maximum_qos{};
                    new_buf_start = decode_qos( new_buf_start, buf_end, &maximum_qos );

                    subscriptions.emplace_back( topic_filter, maximum_qos );
                }
                if ( subscriptions.empty( ) )
                    throw error::malformed_mqtt_packet(
                        "[MQTT-3.8.3-3] A SUBSCRIBE packet MUST contain at least one subscription (topic filter/QoS "
                        "pair)" );

                return std::make_shared<protocol::subscribe>( header, packet_identifier, subscriptions );
            }
        };  // class subscribe_packet_decoder

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
        };  // class subscribe_packet_decoder
    }       // namespace decoder
}  // namespace io_wally
