#include "io_wally/dispatch/dispatcher.hpp"

#include <cassert>
#include <memory>
#include <string>

#include <asio.hpp>

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include "io_wally/dispatch/common.hpp"
#include "io_wally/logging/logging.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/connect_packet.hpp"
#include "io_wally/protocol/puback_packet.hpp"
#include "io_wally/protocol/pubcomp_packet.hpp"
#include "io_wally/protocol/pubrec_packet.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/protocol/unsubscribe_packet.hpp"

namespace io_wally::dispatch
{
    // ------------------------------------------------------------------------------------------------------------
    // Public
    // ------------------------------------------------------------------------------------------------------------

    dispatcher::dispatcher( const context& context, asio::io_service& io_service )
        : session_manager_{context, io_service}
    {
        logger_ = context.logger_factory( ).logger( "dispatcher" );
    }

    void dispatcher::handle_packet_received( const mqtt_packet_sender::packet_container_t::ptr& packet_container )
    {
        logger_->debug( "RX: {}", *packet_container->packet( ) );
        if ( packet_container->packet_type( ) == protocol::packet::Type::CONNECT )
        {
            // For now, we do not support retained LWT messages
            const auto connect = packet_container->packet_as<protocol::connect>( );
            assert( !connect->contains_last_will( ) || !connect->retain_last_will( ) );
            session_manager_.client_connected( packet_container->packet_as<protocol::connect>( ),
                                               packet_container->rx_connection( ) );
        }
        else if ( packet_container->packet_type( ) == protocol::packet::Type::DISCONNECT )
        {
            session_manager_.client_disconnected( packet_container->client_id( ),
                                                  packet_container->disconnect_reason( ) );
        }
        else if ( packet_container->packet_type( ) == protocol::packet::Type::SUBSCRIBE )
        {
            const auto subscribe = packet_container->packet_as<protocol::subscribe>( );
            session_manager_.client_subscribed( packet_container->client_id( ), subscribe );
        }
        else if ( packet_container->packet_type( ) == protocol::packet::Type::UNSUBSCRIBE )
        {
            const auto unsubscribe = packet_container->packet_as<protocol::unsubscribe>( );
            session_manager_.client_unsubscribed( packet_container->client_id( ), unsubscribe );
        }
        else if ( packet_container->packet_type( ) == protocol::packet::Type::PUBLISH )
        {
            const auto publish = packet_container->packet_as<protocol::publish>( );
            session_manager_.client_published( packet_container->client_id( ), publish );
        }
        else if ( packet_container->packet_type( ) == protocol::packet::Type::PUBACK )
        {
            const auto puback = packet_container->packet_as<protocol::puback>( );
            session_manager_.client_acked_publish( packet_container->client_id( ), puback );
        }
        else if ( packet_container->packet_type( ) == protocol::packet::Type::PUBREC )
        {
            const auto pubrec = packet_container->packet_as<protocol::pubrec>( );
            session_manager_.client_received_publish( packet_container->client_id( ), pubrec );
        }
        else if ( packet_container->packet_type( ) == protocol::packet::Type::PUBREL )
        {
            const auto pubrel = packet_container->packet_as<protocol::pubrel>( );
            session_manager_.client_released_publish( packet_container->client_id( ), pubrel );
        }
        else if ( packet_container->packet_type( ) == protocol::packet::Type::PUBCOMP )
        {
            const auto pubcomp = packet_container->packet_as<protocol::pubcomp>( );
            session_manager_.client_completed_publish( packet_container->client_id( ), pubcomp );
        }
        else
        {
            assert( false );
        }
    }

    void dispatcher::client_disconnected_ungracefully( const std::string& client_id,
                                                       dispatch::disconnect_reason reason )
    {
        logger_->info( "Client [{}] disconnected ungracefully: {}", client_id, reason );
        session_manager_.client_disconnected_ungracefully( client_id, reason );
    }

    void dispatcher::stop( const std::string& message )
    {
        logger_->info( "STOPPING: Dispatcher ({}) ...", message );
        session_manager_.destroy_all( );
        logger_->info( "STOPPED:  Dispatcher ({})", message );
    }
}  // namespace io_wally::dispatch
