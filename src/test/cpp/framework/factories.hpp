#pragma once

#include <cstdint>
#include <memory>
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
    std::shared_ptr<io_wally::protocol::subscribe> create_subscribe_packet(
        const std::vector<io_wally::protocol::subscription> subscriptions );

    io_wally::mqtt_packet_sender::packet_container_t::ptr create_subscribe_container(
        const std::vector<io_wally::protocol::subscription> subscriptions );

    std::shared_ptr<io_wally::protocol::publish> create_publish_packet( const std::string& topic, bool retain = false );

    io_wally::mqtt_packet_sender::packet_container_t::ptr create_publish_container( const std::string& topic );
}  // namespace framework
