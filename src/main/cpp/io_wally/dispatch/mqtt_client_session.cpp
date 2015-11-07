#include "io_wally/dispatch/mqtt_client_session.hpp"

#include <string>
#include <memory>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/dispatch/mqtt_client_session_manager.hpp"

namespace io_wally
{
    namespace dispatch
    {
        using namespace std;
        using lvl = boost::log::trivial::severity_level;

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
              tx_in_flight_publications_{session_manager.context( ), session_manager.io_service( ), connection}
        {
        }

        const std::string& mqtt_client_session::client_id( ) const
        {
            return client_id_;
        }

        void mqtt_client_session::send( protocol::mqtt_packet::ptr packet )
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "SEND: " << *packet << " ...";
            if ( auto conn_local = connection_.lock( ) )
            {
                // Connection has not gone away, safe to send
                conn_local->send( packet );
                BOOST_LOG_SEV( logger_, lvl::info ) << "SENT: " << *packet;
            }
            else
            {
                // Connection was closed, destroy this session (IF NOT PERSISTENT)
                BOOST_LOG_SEV( logger_, lvl::info )
                    << "Client connection was asynchronously closed - this session will be destroyed";
                destroy( );
            }
        }

        void mqtt_client_session::publish( std::shared_ptr<protocol::publish> incoming_publish )
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "PUBLISH:   " << *incoming_publish << " ...";
            tx_in_flight_publications_.publish_received( incoming_publish );
            BOOST_LOG_SEV( logger_, lvl::debug ) << "PUBLISHED: " << *incoming_publish;
        }

        void mqtt_client_session::client_acked_publish( std::shared_ptr<protocol::puback> puback )
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "ACK:   " << *puback << " ...";
            tx_in_flight_publications_.response_received( puback );
            BOOST_LOG_SEV( logger_, lvl::debug ) << "ACKED: " << *puback;
        }

        void mqtt_client_session::destroy( )
        {
            session_manager_.destroy( client_id_ );
        }
    }  // namespace dispatch
}  // namespace io_wally
