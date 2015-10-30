#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "framework/mocks.hpp"

#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/subscription.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/dispatch/common.hpp"

namespace framework
{
    io_wally::mqtt_packet_sender::packet_container_t::ptr create_subscribe_container(
        const std::vector<io_wally::protocol::subscription> subscriptions );

    io_wally::mqtt_packet_sender::packet_container_t::ptr create_subscribe_container(
        const std::vector<io_wally::protocol::subscription> subscriptions )
    {
        auto header = io_wally::protocol::packet::header{0x08 << 4, 20};  // 20 is just some number
        auto pktid = uint16_t{9};

        auto subscribe_ptr = std::make_shared<const io_wally::protocol::subscribe>( header, pktid, subscriptions );

        auto sender_ptr = framework::packet_sender_mock::create( );

        return io_wally::mqtt_packet_sender::packet_container_t::subscribe_packet(
            "client_mock", sender_ptr, subscribe_ptr );
    }

    io_wally::mqtt_packet_sender::packet_container_t::ptr create_publish_container( const std::string& topic );

    io_wally::mqtt_packet_sender::packet_container_t::ptr create_publish_container( const std::string& topic )
    {
        auto const header = io_wally::protocol::packet::header{( 3 << 4 ) | 0x00, 20};
        auto const pktid = std::uint16_t{7};
        auto const msg = std::vector<uint8_t>{'t', 'e', 's', 't'};

        auto publish_ptr = std::make_shared<const io_wally::protocol::publish>( header, topic, pktid, msg );

        auto sender_ptr = framework::packet_sender_mock::create( );

        return io_wally::mqtt_packet_sender::packet_container_t::publish_packet(
            "client_mock", sender_ptr, publish_ptr );
    }
}  // namespace framework
