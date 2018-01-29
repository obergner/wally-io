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

namespace io_wally
{
    namespace dispatch
    {
        using namespace std;
        // ------------------------------------------------------------------------------------------------------------
        // Public
        // ------------------------------------------------------------------------------------------------------------

        mqtt_client_session::ptr mqtt_client_session::create( mqtt_client_session_manager& session_manager,
                                                              const std::string& client_id,
                                                              std::weak_ptr<mqtt_packet_sender> connection )
        {
            return std::make_shared<mqtt_client_session>( session_manager, client_id, connection );
        }

        mqtt_client_session::mqtt_client_session( mqtt_client_session_manager& session_manager,
                                                  const std::string& client_id,
                                                  std::weak_ptr<mqtt_packet_sender> connection )
            : session_manager_{session_manager},
              client_id_{client_id},
              connection_{connection},
              tx_in_flight_publications_{session_manager.context( ), session_manager.io_service( ), connection},
              rx_in_flight_publications_{session_manager.context( ), session_manager.io_service( ), connection}
        {
            logger_ = session_manager_.context( ).logger_factory( ).logger( "session/" + client_id_ );
        }

        const std::string& mqtt_client_session::client_id( ) const
        {
            return client_id_;
        }

        void mqtt_client_session::send( protocol::mqtt_packet::ptr packet )
        {
            if ( auto conn_local = connection_.lock( ) )
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

        void mqtt_client_session::publish( std::shared_ptr<protocol::publish> incoming_publish,
                                           const protocol::packet::QoS maximum_qos )
        {
            tx_in_flight_publications_.publish( incoming_publish, maximum_qos );
            logger_->debug( "PUBLSIHED: {} (maxqos: {})", *incoming_publish, maximum_qos );
        }

        void mqtt_client_session::client_sent_publish( std::shared_ptr<protocol::publish> incoming_publish )
        {
            bool dispatch_publish = rx_in_flight_publications_.client_sent_publish( incoming_publish );
            if ( dispatch_publish )
            {
                session_manager_.publish( incoming_publish );
            }
            else
            {
                logger_->info( "Client re-sent PUBLISH packet {} - will be ignored", *incoming_publish );
            }
        }

        void mqtt_client_session::client_acked_publish( std::shared_ptr<protocol::puback> puback )
        {
            tx_in_flight_publications_.response_received( puback );
            logger_->debug( "ACKED: {}", *puback );
        }

        void mqtt_client_session::client_received_publish( std::shared_ptr<protocol::pubrec> pubrec )
        {
            tx_in_flight_publications_.response_received( pubrec );
            logger_->debug( "RCVD: {}", *pubrec );
        }

        void mqtt_client_session::client_released_publish( std::shared_ptr<protocol::pubrel> pubrel )
        {
            rx_in_flight_publications_.client_sent_pubrel( pubrel );
            logger_->debug( "RCVD: {}", *pubrel );
        }

        void mqtt_client_session::client_completed_publish( std::shared_ptr<protocol::pubcomp> pubcomp )
        {
            tx_in_flight_publications_.response_received( pubcomp );
            logger_->debug( "CMPD: {}", *pubcomp );
        }

        void mqtt_client_session::destroy( )
        {
            session_manager_.destroy( client_id_ );
        }
    }  // namespace dispatch
}  // namespace io_wally
