#include "io_wally/dispatch/mqtt_client_session_manager.hpp"

#include <string>
#include <memory>
#include <map>
#include <algorithm>

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/context.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/protocol/suback_packet.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/puback_packet.hpp"
#include "io_wally/protocol/pubrec_packet.hpp"
#include "io_wally/protocol/pubcomp_packet.hpp"
#include "io_wally/dispatch/common.hpp"
#include "io_wally/dispatch/mqtt_client_session.hpp"
#include "io_wally/dispatch/topic_subscriptions.hpp"

namespace io_wally
{
    namespace dispatch
    {
        using namespace std;
        using lvl = boost::log::trivial::severity_level;

        // ------------------------------------------------------------------------------------------------------------
        // Public
        // ------------------------------------------------------------------------------------------------------------

        mqtt_client_session_manager::mqtt_client_session_manager( const io_wally::context& context,
                                                                  boost::asio::io_service& io_service )
            : context_{context}, io_service_{io_service}
        {
        }

        mqtt_client_session_manager::~mqtt_client_session_manager( )
        {
            destroy_all( );
        }

        const io_wally::context& mqtt_client_session_manager::context( ) const
        {
            return context_;
        }

        boost::asio::io_service& mqtt_client_session_manager::io_service( ) const
        {
            return io_service_;
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

        void mqtt_client_session_manager::client_disconnected( const std::string& client_id,
                                                               const dispatch::disconnect_reason reason )
        {
            sessions_.erase( client_id );
            BOOST_LOG_SEV( logger_, lvl::debug ) << "Client disconnected: [cltid:" << client_id << "|rsn:" << reason
                                                 << "] - session destroyed";
        }

        void mqtt_client_session_manager::client_subscribed( const std::string& client_id,
                                                             std::shared_ptr<protocol::subscribe> subscribe )
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "SUBSCRIBE:  [cltid:" << client_id << "|pkt:" << *subscribe
                                                 << "] ...";
            auto suback = topic_subscriptions_.subscribe( client_id, subscribe );
            // TODO: This will default construct (is that possible?) a new session if client_id is not yet registered.
            auto session = sessions_[client_id];
            session->send( suback );
            BOOST_LOG_SEV( logger_, lvl::debug ) << "SUBSCRIBED: [cltid:" << client_id << "|pkt:" << *subscribe << "]";
        }

        void mqtt_client_session_manager::client_published( const std::string& client_id,
                                                            std::shared_ptr<protocol::publish> incoming_publish )
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "PUBLISH:   [cltid:" << client_id << "|pkt:" << *incoming_publish
                                                 << "] ...";

            auto puback = std::make_shared<protocol::puback>( incoming_publish->packet_identifier( ) );
            // TODO: This will default construct (is that possible?) a new session if client_id is not yet registered.
            auto session = sessions_[client_id];
            session->send( puback );

            auto resolved_subscribers = topic_subscriptions_.resolve_subscribers( incoming_publish );
            for_each( resolved_subscribers.begin( ),
                      resolved_subscribers.end( ),
                      [this, &incoming_publish]( const resolved_subscriber_t& subscriber )
                      {
                // TODO: This will default construct (is that possible?) a new session if client_id is not yet
                // registered.
                auto session = sessions_[subscriber.first];
                session->publish( incoming_publish, subscriber.second );
            } );
            BOOST_LOG_SEV( logger_, lvl::debug ) << "PUBLISHED: [cltid:" << client_id << "|pkt:" << *incoming_publish
                                                 << "]";
        }

        void mqtt_client_session_manager::client_acked_publish( const std::string& client_id,
                                                                std::shared_ptr<protocol::puback> puback )
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "RX ACK: [cltid:" << client_id << "|pkt:" << *puback << "] ...";
            // TODO: This will default construct (is that possible?) a new session if client_id is not yet registered.
            auto session = sessions_[client_id];
            session->client_acked_publish( puback );
            BOOST_LOG_SEV( logger_, lvl::debug ) << "RX ACK: [cltid:" << client_id << "|pkt:" << *puback << "]";
        }

        void mqtt_client_session_manager::client_received_publish( const std::string& client_id,
                                                                   std::shared_ptr<protocol::pubrec> pubrec )
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "RX REC: [cltid:" << client_id << "|pkt:" << *pubrec << "] ...";
            // TODO: This will default construct (is that possible?) a new session if client_id is not yet registered.
            auto session = sessions_[client_id];
            session->client_received_publish( pubrec );
            BOOST_LOG_SEV( logger_, lvl::debug ) << "RX REC: [cltid:" << client_id << "|pkt:" << *pubrec << "]";
        }

        void mqtt_client_session_manager::client_completed_publish( const std::string& client_id,
                                                                    std::shared_ptr<protocol::pubcomp> pubcomp )
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "RX COMP: [cltid:" << client_id << "|pkt:" << *pubcomp << "] ...";
            // TODO: This will default construct (is that possible?) a new session if client_id is not yet registered.
            auto session = sessions_[client_id];
            session->client_completed_publish( pubcomp );
            BOOST_LOG_SEV( logger_, lvl::debug ) << "RX COMP: [cltid:" << client_id << "|pkt:" << *pubcomp << "]";
        }

        void mqtt_client_session_manager::destroy( const std::string& client_id )
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
