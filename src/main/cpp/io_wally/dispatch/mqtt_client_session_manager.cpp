#include "io_wally/dispatch/mqtt_client_session_manager.hpp"

#include <string>
#include <memory>
#include <map>
#include <algorithm>

#include <boost/log/trivial.hpp>

#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/dispatch/common.hpp"
#include "io_wally/dispatch/mqtt_client_session.hpp"

namespace io_wally
{
    namespace dispatch
    {
        using namespace std;
        using lvl = boost::log::trivial::severity_level;

        // ------------------------------------------------------------------------------------------------------------
        // Public
        // ------------------------------------------------------------------------------------------------------------

        mqtt_client_session_manager::~mqtt_client_session_manager( )
        {
            destroy_all( );
        }

        void mqtt_client_session_manager::client_connected( const std::string& client_id,
                                                            std::weak_ptr<mqtt_packet_sender> connection )
        {
            auto connection_ptr = shared_ptr<mqtt_packet_sender>( connection );
            if ( connection_ptr )
            {
                BOOST_LOG_SEV( logger_, lvl::debug ) << "Client connected: [cltid:" << client_id
                                                     << "|conn:" << *connection_ptr << "]";
                // TODO: Maybe we shouldn't pass a REFERENCE to this, since we might go away (likewise in
                // mqtt_packet_sender)
                auto session = mqtt_client_session::create( *this, client_id, connection );
                sessions_.emplace( client_id, session );
                BOOST_LOG_SEV( logger_, lvl::info ) << "Session for client [cltid:" << client_id
                                                    << "|conn:" << *connection_ptr
                                                    << "] created [total:" << sessions_.size( ) << "]";
            }
            else
            {
                BOOST_LOG_SEV( logger_, lvl::warning )
                    << "Client connected [cltid:" << client_id
                    << "], yet connection was immediately closed (network/protocol error)";
            }
        }

        void mqtt_client_session_manager::client_disconnected( const std::string client_id,
                                                               const dispatch::disconnect_reason reason )
        {
            sessions_.erase( client_id );
            BOOST_LOG_SEV( logger_, lvl::debug ) << "Client disconnected: [cltid:" << client_id << "|rsn:" << reason
                                                 << "] - session destroyed";
        }

        void mqtt_client_session_manager::send( const std::string& client_id, protocol::mqtt_packet::ptr packet )
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "SEND: [cltid:" << client_id << "|pkt:" << *packet << "] ...";
            // TODO: This will default construct (is that possible?) a new session if client_id is not yet registered.
            auto session = sessions_[client_id];
            session->send( packet );
            BOOST_LOG_SEV( logger_, lvl::debug ) << "SENT: [cltid:" << client_id << "|pkt:" << *packet << "]";
        }

        void mqtt_client_session_manager::publish( const std::vector<resolved_subscriber_t>& subscribers,
                                                   std::shared_ptr<const protocol::publish> publish )
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "PUBLISH:   [subscr:" << subscribers.size( ) << "|pkt:" << *publish
                                                 << "] ...";
            for_each( subscribers.begin( ),
                      subscribers.end( ),
                      [this, &publish]( const resolved_subscriber_t& subscriber )
                      {
                // TODO: This will default construct (is that possible?) a new session if client_id is not yet
                // registered.
                auto session = sessions_[subscriber.first];
                session->send( publish );
            } );
            BOOST_LOG_SEV( logger_, lvl::debug ) << "PUBLISHED: [subscr:" << subscribers.size( ) << "|pkt:" << *publish
                                                 << "]";
        }

        void mqtt_client_session_manager::destroy( const std::string client_id )
        {
            auto rem_cnt = sessions_.erase( client_id );
            if ( rem_cnt > 0 )
                BOOST_LOG_SEV( logger_, lvl::info ) << "Client session [cltid:" << client_id << "] destroyed";
        }

        void mqtt_client_session_manager::destroy_all( )
        {
            auto sess_cnt = sessions_.size( );
            sessions_.clear( );
            BOOST_LOG_SEV( logger_, lvl::info ) << "SHUTDOWN: [" << sess_cnt << "] client session(s) destroyed";
        }
    }  // namespace dispatch
}  // namespace io_wally
