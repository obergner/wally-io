#include "io_wally/dispatch/mqtt_client_session.hpp"

#include <memory>
#include <string>

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include "io_wally/dispatch/mqtt_client_session_manager.hpp"
#include "io_wally/logging/logging.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/puback_packet.hpp"
#include "io_wally/protocol/pubcomp_packet.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/pubrec_packet.hpp"

namespace io_wally::dispatch
{
    // ------------------------------------------------------------------------------------------------------------
    // Public
    // ------------------------------------------------------------------------------------------------------------

    auto mqtt_client_session::client_connected( mqtt_client_session_manager& session_manager,
                                                std::shared_ptr<protocol::connect> connect,
                                                std::weak_ptr<mqtt_packet_sender> connection )
        -> mqtt_client_session::ptr
    {
        return std::make_shared<mqtt_client_session>( session_manager, connect, connection );
    }

    mqtt_client_session::mqtt_client_session( mqtt_client_session_manager& session_manager,
                                              const std::shared_ptr<protocol::connect>& connect,
                                              const std::weak_ptr<mqtt_packet_sender>& connection )
        : session_manager_{session_manager},
          client_id_{connect->client_id( )},
          connection_{connection},
          tx_in_flight_publications_{session_manager.context( ), session_manager.io_service( ), connection},
          rx_in_flight_publications_{session_manager.context( ), session_manager.io_service( ), connection}
    {
        lwt_message_ = connect->contains_last_will( ) ? connect : nullptr;
        logger_ = session_manager_.context( ).logger_factory( ).logger( "session/" + client_id_ );
    }

    auto mqtt_client_session::client_id( ) const -> const std::string&
    {
        return client_id_;
    }

    void mqtt_client_session::send( const protocol::mqtt_packet::ptr& packet )
    {
        if ( const auto conn_local = connection_.lock( ) )
        {
            // Connection has not gone away, safe to send
            conn_local->send( packet );
            logger_->info( "SENT: {}", *packet );
        }
        else
        {
            // Connection was closed, destroy this session (IF NOT PERSISTENT)
            logger_->info( "Client connection was asynchronously closed - this session will be destroyed" );
            destroy( );
        }
    }

    void mqtt_client_session::publish( const std::shared_ptr<protocol::publish>& incoming_publish,
                                       const protocol::packet::QoS maximum_qos )
    {
        tx_in_flight_publications_.publish( incoming_publish, maximum_qos );
        logger_->debug( "PUBLISHED: {} (maxqos: {})", *incoming_publish, maximum_qos );
    }

    void mqtt_client_session::client_sent_publish( const std::shared_ptr<protocol::publish>& incoming_publish )
    {
        const bool dispatch_publish = rx_in_flight_publications_.client_sent_publish( incoming_publish );
        if ( dispatch_publish )
        {
            session_manager_.publish( incoming_publish );
        }
        else
        {
            logger_->info( "Client re-sent PUBLISH packet {} - will be ignored", *incoming_publish );
        }
    }

    void mqtt_client_session::client_acked_publish( const std::shared_ptr<protocol::puback>& puback )
    {
        tx_in_flight_publications_.response_received( puback );
        logger_->debug( "ACKED: {}", *puback );
    }

    void mqtt_client_session::client_received_publish( const std::shared_ptr<protocol::pubrec>& pubrec )
    {
        tx_in_flight_publications_.response_received( pubrec );
        logger_->debug( "RCVD: {}", *pubrec );
    }

    void mqtt_client_session::client_released_publish( const std::shared_ptr<protocol::pubrel>& pubrel )
    {
        rx_in_flight_publications_.client_sent_pubrel( pubrel );
        logger_->debug( "RCVD: {}", *pubrel );
    }

    void mqtt_client_session::client_completed_publish( const std::shared_ptr<protocol::pubcomp>& pubcomp )
    {
        tx_in_flight_publications_.response_received( pubcomp );
        logger_->debug( "CMPD: {}", *pubcomp );
    }

    void mqtt_client_session::client_disconnected_ungracefully( dispatch::disconnect_reason reason )
    {
        logger_->warn( "UNGRACEFUL DISCONNECT: [{}:{}]", client_id_, reason );
        if ( lwt_message_ )
        {
            logger_->debug( "SEND: client [{}]'s LWT message ...", client_id_ );
            auto lwt_publish =
                protocol::publish::create( false, lwt_message_->last_will_qos( ), false, *lwt_message_->will_topic( ),
                                           0x0000, *lwt_message_->will_message( ) );
            session_manager_.publish( std::move( lwt_publish ) );
            logger_->info( "SENT: client [{}]'s LWT message", client_id_ );
        }
    }

    void mqtt_client_session::destroy( )
    {
        session_manager_.destroy( client_id_ );
    }
}  // namespace io_wally::dispatch
