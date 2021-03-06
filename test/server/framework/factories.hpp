#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <cxxopts.hpp>

#include "framework/mocks.hpp"

#include "io_wally/app/options_factory.hpp"
#include "io_wally/context.hpp"
#include "io_wally/dispatch/common.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/protocol/subscription.hpp"
#include "io_wally/protocol/unsubscribe_packet.hpp"

namespace framework
{
    static const io_wally::app::options_factory OPTIONS_FACTORY{};

    static cxxopts::Options PROGRAM_OPTIONS = OPTIONS_FACTORY.create( );

    std::shared_ptr<io_wally::protocol::connect> create_connect_packet( const std::string& client_id,
                                                                        bool clean_session = false );

    std::shared_ptr<io_wally::protocol::connect> create_connect_packet_with_lwt(
        const std::string& client_id,
        bool clean_session = false,
        const std::string& will_topic = "/#",
        const std::string& will_message = "last will and testament",
        io_wally::protocol::packet::QoS will_qos = io_wally::protocol::packet::QoS::AT_MOST_ONCE,
        bool will_retain = false );

    std::shared_ptr<io_wally::protocol::subscribe> create_subscribe_packet(
        const std::vector<io_wally::protocol::subscription> subscriptions );

    std::shared_ptr<io_wally::protocol::unsubscribe> create_unsubscribe_packet(
        const std::vector<std::string> topic_filters );

    io_wally::mqtt_packet_sender::packet_container_t::ptr create_subscribe_container(
        const std::vector<io_wally::protocol::subscription> subscriptions );

    std::shared_ptr<io_wally::protocol::publish> create_publish_packet( const std::string& topic,
                                                                        bool retain = false,
                                                                        const std::vector<std::uint8_t> msg = {
                                                                            't', 'e', 's', 't'} );

    io_wally::mqtt_packet_sender::packet_container_t::ptr create_publish_container( const std::string& topic );

    cxxopts::ParseResult create_parse_result( );

    io_wally::context create_context( );
}  // namespace framework
