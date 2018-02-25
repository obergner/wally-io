#include "framework/factories.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <cxxopts.hpp>

#include "framework/mocks.hpp"

#include "io_wally/dispatch/common.hpp"
#include "io_wally/impl/accept_all_authentication_service_factory.hpp"
#include "io_wally/logging/logging.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/protocol/subscription.hpp"

namespace framework
{
    std::shared_ptr<io_wally::protocol::connect> create_connect_packet( const std::string& client_id,
                                                                        bool clean_session )
    {
        const auto remaining_length = uint32_t{20};  // 20 is just some number
        const auto prot_name = "MQTT";
        const auto prot_level = std::uint8_t{0x04};
        auto connect_flags = std::uint8_t{0x00};
        connect_flags = clean_session ? connect_flags | 0x02 : connect_flags;
        const auto keep_alive_secs = std::uint16_t{0x0000};
        const auto will_topic = nullptr;
        const auto will_message = std::vector<uint8_t>{};
        const auto username = nullptr;
        const auto password = nullptr;

        return std::make_shared<io_wally::protocol::connect>( remaining_length, prot_name, prot_level, connect_flags,
                                                              keep_alive_secs, client_id.c_str( ), will_topic,
                                                              will_message, username, password );
    }

    std::shared_ptr<io_wally::protocol::connect> create_connect_packet_with_lwt(
        const std::string& client_id,
        bool clean_session,
        const std::string& will_topic,
        const std::string& will_message,
        io_wally::protocol::packet::QoS will_qos,
        bool will_retain )
    {
        const auto remaining_length = uint32_t{20};  // 20 is just some number
        const auto prot_name = "MQTT";
        const auto prot_level = std::uint8_t{0x04};
        auto connect_flags = std::uint8_t{0x00};
        connect_flags = clean_session ? connect_flags | 0x02 : connect_flags;
        connect_flags |= 0x04;
        connect_flags |= ( static_cast<uint8_t>( will_qos ) << 3 );
        connect_flags = will_retain ? connect_flags | 0x20 : connect_flags;
        const auto keep_alive_secs = std::uint16_t{0x0000};
        const auto username = nullptr;
        const auto password = nullptr;

        return std::make_shared<io_wally::protocol::connect>(
            remaining_length, prot_name, prot_level, connect_flags, keep_alive_secs, client_id.c_str( ),
            will_topic.c_str( ), std::vector<uint8_t>{will_message.begin( ), will_message.end( )}, username, password );
    }

    std::shared_ptr<io_wally::protocol::subscribe> create_subscribe_packet(
        const std::vector<io_wally::protocol::subscription> subscriptions )
    {
        const auto remaining_length = uint32_t{20};  // 20 is just some number
        const auto pktid = uint16_t{9};

        return std::make_shared<io_wally::protocol::subscribe>( remaining_length, pktid, subscriptions );
    }

    std::shared_ptr<io_wally::protocol::unsubscribe> create_unsubscribe_packet(
        const std::vector<std::string> topic_filters )
    {
        const auto remaining_length = uint32_t{20};  // 20 is just some number
        const auto pktid = uint16_t{11};

        return std::make_shared<io_wally::protocol::unsubscribe>( remaining_length, pktid, topic_filters );
    }

    io_wally::mqtt_packet_sender::packet_container_t::ptr create_subscribe_container(
        const std::vector<io_wally::protocol::subscription> subscriptions )
    {
        auto subscribe_ptr = create_subscribe_packet( subscriptions );
        auto sender_ptr = framework::packet_sender_mock::create( );

        return io_wally::mqtt_packet_sender::packet_container_t::contain( "client_mock", sender_ptr, subscribe_ptr );
    }

    std::shared_ptr<io_wally::protocol::publish> create_publish_packet( const std::string& topic,
                                                                        bool retain,
                                                                        const std::vector<uint8_t> msg )
    {
        auto flags = std::uint8_t{0x00};
        if ( retain )
        {
            flags |= 0x01;
        }

        const auto type_and_flags = std::uint8_t( ( 3 << 4 ) | flags );
        const auto remaining_length = uint32_t{20};  // 20 is just some number
        const auto pktid = std::uint16_t{7};

        return std::make_shared<io_wally::protocol::publish>( type_and_flags, remaining_length, topic, pktid, msg );
    }

    io_wally::mqtt_packet_sender::packet_container_t::ptr create_publish_container( const std::string& topic )
    {
        auto publish_ptr = create_publish_packet( topic );
        auto sender_ptr = framework::packet_sender_mock::create( );

        return io_wally::mqtt_packet_sender::packet_container_t::contain( "client_mock", sender_ptr, publish_ptr );
    }

    cxxopts::ParseResult create_parse_result( )
    {
        const char* command_line_args[]{"executable"};
        auto argc = static_cast<int>( sizeof( command_line_args ) / sizeof( *command_line_args ) );
        auto argv = const_cast<char**>( command_line_args );

        return PROGRAM_OPTIONS.parse( argc, argv );
    }

    io_wally::context create_context( )
    {
        return io_wally::context( create_parse_result( ),
                                  std::unique_ptr<io_wally::spi::authentication_service>(
                                      new io_wally::impl::accept_all_authentication_service{} ),
                                  io_wally::logging::logger_factory::disabled( ) );
    }
}  // namespace framework
