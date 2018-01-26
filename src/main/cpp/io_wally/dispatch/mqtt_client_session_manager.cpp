#include "io_wally/dispatch/mqtt_client_session_manager.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <string>

#include <asio.hpp>

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include "io_wally/context.hpp"
#include "io_wally/dispatch/common.hpp"
#include "io_wally/dispatch/mqtt_client_session.hpp"
#include "io_wally/dispatch/topic_subscriptions.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/puback_packet.hpp"
#include "io_wally/protocol/pubcomp_packet.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/pubrec_packet.hpp"
#include "io_wally/protocol/suback_packet.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/protocol/unsuback_packet.hpp"
#include "io_wally/protocol/unsubscribe_packet.hpp"

namespace io_wally
{
    namespace dispatch
    {
        using namespace std;

        // ------------------------------------------------------------------------------------------------------------
        // Public
        // ------------------------------------------------------------------------------------------------------------

        mqtt_client_session_manager::mqtt_client_session_manager( const io_wally::context& context,
                                                                  ::asio::io_service& io_service )
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

        ::asio::io_service& mqtt_client_session_manager::io_service( ) const
        {
            return io_service_;
        }

        void mqtt_client_session_manager::client_connected( const std::string& client_id,
                                                            std::weak_ptr<mqtt_packet_sender> connection )
        {
            if ( auto connection_ptr = connection.lock( ) )
            {
                // TODO: Maybe we shouldn't pass a REFERENCE to this, since we might go away (likewise in
                // mqtt_packet_sender)
                auto session = mqtt_client_session::create( *this, client_id, connection );
                sessions_.emplace( client_id, session );
                logger_->info( "Session for client [cltid:{}|conn:{}] created [total:{}]", client_id, *connection_ptr,
                               sessions_.size( ) );
            }
            else
            {
                logger_->warn(
                    "Client connected [cltid:{}], yet session was immediately closed (network/protocol error)",
                    client_id );
            }
        }

        void mqtt_client_session_manager::client_disconnected( const std::string& client_id,
                                                               const dispatch::disconnect_reason reason )
        {
            sessions_.erase( client_id );
            logger_->debug( "Client disconnected: [cltid:{}|rsn:{}] - session destroyed", client_id, reason );
        }

        void mqtt_client_session_manager::client_subscribed( const std::string& client_id,
                                                             std::shared_ptr<protocol::subscribe> subscribe )
        {
            auto suback = topic_subscriptions_.subscribe( client_id, subscribe );
            if ( auto session = sessions_[client_id] )
            {
                // TODO: mqtt_client_session exposes an event-oriented interface, i.e. client code (as this code) tells
                // it
                // what has happened, not what to do. This "send()" method is the only exception. Can we get rid of it?
                session->send( suback );
            }
            logger_->debug( "SUBSCRIBED: [cltid:{}|pkt:{}]", client_id, *subscribe );
        }

        void mqtt_client_session_manager::client_unsubscribed( const std::string& client_id,
                                                               std::shared_ptr<protocol::unsubscribe> unsubscribe )
        {
            auto unsuback = topic_subscriptions_.unsubscribe( client_id, unsubscribe );
            if ( auto session = sessions_[client_id] )
            {
                // TODO: mqtt_client_session exposes an event-oriented interface, i.e. client code (as this code) tells
                // it
                // what has happened, not what to do. This "send()" method is the only exception. Can we get rid of it?
                session->send( unsuback );
            }
            logger_->debug( "UNSUBSCRIBED: [cltid:{}|pkt:{}]", client_id, *unsubscribe );
        }

        void mqtt_client_session_manager::client_published( const std::string& client_id,
                                                            std::shared_ptr<protocol::publish> incoming_publish )
        {
            logger_->debug( "RX PUBLISH: [cltid:{}|pkt:{}]", client_id, *incoming_publish );
            if ( incoming_publish->retain( ) )
            {
                retained_messages_.retain( incoming_publish );
                // [MQTT-3.3.1.3] PUBLISH packets forwarded to subscriptions that already existed when they were
                // published MUST have their retain flag set to 0
                incoming_publish->retain( false );
                logger_->debug( "RETAINED: [topic:{}|size:{}]", incoming_publish->topic( ),
                                incoming_publish->application_message( ).size( ) );
            }
            if ( auto session = sessions_[client_id] )
            {
                session->client_sent_publish( incoming_publish );
            }
        }

        void mqtt_client_session_manager::client_acked_publish( const std::string& client_id,
                                                                std::shared_ptr<protocol::puback> puback )
        {
            logger_->debug( "RX ACK: [cltid:{}|pkt:{}]", client_id, *puback );
            // TODO: This will default construct (is that possible?) a new session if client_id is not yet
            // registered.
            if ( auto session = sessions_[client_id] )
            {
                session->client_acked_publish( puback );
            }
        }

        void mqtt_client_session_manager::client_received_publish( const std::string& client_id,
                                                                   std::shared_ptr<protocol::pubrec> pubrec )
        {
            logger_->debug( "RX REC: [cltid:{}|pkt:{}]", client_id, *pubrec );
            // TODO: This will default construct (is that possible?) a new session if client_id is not yet registered.
            if ( auto session = sessions_[client_id] )
            {
                session->client_received_publish( pubrec );
            }
        }

        void mqtt_client_session_manager::client_released_publish( const std::string& client_id,
                                                                   std::shared_ptr<protocol::pubrel> pubrel )
        {
            logger_->debug( "RX REL: [cltid:{}|pkt:{}]", client_id, *pubrel );
            // TODO: This will default construct (is that possible?) a new session if client_id is not yet registered.
            if ( auto session = sessions_[client_id] )
            {
                session->client_released_publish( pubrel );
            }
        }

        void mqtt_client_session_manager::client_completed_publish( const std::string& client_id,
                                                                    std::shared_ptr<protocol::pubcomp> pubcomp )
        {
            logger_->debug( "RX COMP: [cltid:{}|pkt:{}]", client_id, *pubcomp );
            // TODO: This will default construct (is that possible?) a new session if client_id is not yet registered.
            if ( auto session = sessions_[client_id] )
            {
                session->client_completed_publish( pubcomp );
            }
        }

        void mqtt_client_session_manager::destroy( const std::string& client_id )
        {
            auto rem_cnt = sessions_.erase( client_id );
            if ( rem_cnt > 0 )
                logger_->info( "Client session [cltid:{}] destroyed", client_id );
        }

        void mqtt_client_session_manager::destroy_all( )
        {
            auto sess_cnt = sessions_.size( );
            sessions_.clear( );
            logger_->info( "SHUTDOWN: [{}] client session(s) destroyed", sess_cnt );
        }

        // ------------------------------------------------------------------------------------------------------------
        // Private
        // ------------------------------------------------------------------------------------------------------------

        void mqtt_client_session_manager::publish( std::shared_ptr<protocol::publish> incoming_publish )
        {
            auto resolved_subscribers = topic_subscriptions_.resolve_subscribers( incoming_publish );
            for_each( resolved_subscribers.begin( ), resolved_subscribers.end( ),
                      [this, &incoming_publish]( const resolved_subscriber_t& subscriber ) {
                          // TODO: This will default construct (is that possible?) a new session if client_id is not yet
                          // registered.
                          if ( auto session = sessions_[subscriber.first] )
                          {
                              session->publish( incoming_publish, subscriber.second );
                          }
                      } );
        }
    }  // namespace dispatch
}  // namespace io_wally
