#include "io_wally/dispatch/mqtt_client_session_manager.hpp"

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
        // ------------------------------------------------------------------------------------------------------------
        // mqtt_client_session_manager::session_store: public
        // ------------------------------------------------------------------------------------------------------------

        mqtt_client_session_manager::session_store::session_store( mqtt_client_session_manager& parent )
            : parent_{parent}
        {
        }

        mqtt_client_session::ptr mqtt_client_session_manager::session_store::operator[](
            const std::string& client_id ) const
        {
            if ( const auto pos = sessions_.find( client_id ); pos != std::end( sessions_ ) )
                return pos->second;
            else
                return mqtt_client_session::ptr{};
        }

        bool mqtt_client_session_manager::session_store::insert( std::weak_ptr<mqtt_packet_sender> connection )
        {
            if ( const auto locked_connection = connection.lock( ) )
            {
                assert( locked_connection->client_id( ) );

                auto stored = false;
                auto pos = std::map<const std::string, mqtt_client_session::ptr>::iterator{};
                std::tie( pos, stored ) = sessions_.try_emplace(
                    *locked_connection->client_id( ),
                    mqtt_client_session::create( parent_, *locked_connection->client_id( ), connection ) );

                // We touch both stored AND pos since otherwise clang-analyzer complains about a dead store to pos.
                return stored && ( pos != sessions_.end( ) );
            }

            return false;
        }

        void mqtt_client_session_manager::session_store::remove( const std::string& client_id )
        {
            sessions_.erase( client_id );
        }

        void mqtt_client_session_manager::session_store::clear( )
        {
            sessions_.clear( );
        }

        std::size_t mqtt_client_session_manager::session_store::size( ) const
        {
            return sessions_.size( );
        }

        // ------------------------------------------------------------------------------------------------------------
        // mqtt_client_session_manager: public
        // ------------------------------------------------------------------------------------------------------------

        mqtt_client_session_manager::mqtt_client_session_manager( const io_wally::context& context,
                                                                  asio::io_service& io_service )
            : context_{context}, io_service_{io_service}, topic_subscriptions_{context}
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

        asio::io_service& mqtt_client_session_manager::io_service( ) const
        {
            return io_service_;
        }

        void mqtt_client_session_manager::client_connected( const std::string& client_id,
                                                            std::weak_ptr<mqtt_packet_sender> connection )
        {
            if ( sessions_.insert( connection ) )
            {
                logger_->info( "Session for client [cltid:{}] created [total:{}]", client_id, sessions_.size( ) );
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
            sessions_.remove( client_id );
            logger_->debug( "Client disconnected: [cltid:{}|rsn:{}] - session destroyed", client_id, reason );
        }

        void mqtt_client_session_manager::client_subscribed( const std::string& client_id,
                                                             std::shared_ptr<protocol::subscribe> subscribe )
        {
            if ( const auto suback = topic_subscriptions_.subscribe( client_id, subscribe );
                 const auto session = sessions_[client_id] )
            {
                // TODO: mqtt_client_session exposes an event-oriented interface, i.e. client code (as this code) tells
                // it what has happened, not what to do. This "send()" method is the only exception. Can we get rid of
                // it?
                session->send( suback );

                const auto matching_retained_messages = retained_messages_.messages_for( subscribe );
                for ( const auto retained_message : matching_retained_messages )
                {
                    assert( retained_message.first->retain( ) );
                    session->publish( retained_message.first, retained_message.second );
                }

                logger_->debug( "SUBSCRIBED: [cltid:{}|pkt:{}] - received [{}] retained message(s)", client_id,
                                *subscribe, matching_retained_messages.size( ) );
            }
        }

        void mqtt_client_session_manager::client_unsubscribed( const std::string& client_id,
                                                               std::shared_ptr<protocol::unsubscribe> unsubscribe )
        {
            if ( const auto unsuback = topic_subscriptions_.unsubscribe( client_id, unsubscribe );
                 const auto session = sessions_[client_id] )
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
            const auto session = sessions_[client_id];
            if ( !session )
            {
                logger_->warn( "No session for client [{}] - session asynchronously closed?", client_id );
                return;
            }

            if ( incoming_publish->retain( ) )
            {
                // [MQTT-3.3.1.3] PUBLISH packets forwarded to subscriptions that already existed when they were
                // published MUST have their retain flag set to 0
                incoming_publish->retain( false );
                session->client_sent_publish( incoming_publish );
                // Retain incoming publish only AFTER it has been published to all interested subscribers with retained
                // flag set to 0. Otherwise, we will store it in retained_messages_ with a retained flag initially set
                // to 0, and a concurrently subcribing client may pull it out of retained_messages_ BEFORE its retained
                // flag could be set to 1 again.
                incoming_publish->retain( true );
                retained_messages_.retain( incoming_publish );
                logger_->debug( "RETAINED: [topic:{}|size:{}]", incoming_publish->topic( ),
                                incoming_publish->application_message( ).size( ) );
            }
            else
            {
                session->client_sent_publish( incoming_publish );
            }
        }

        void mqtt_client_session_manager::client_acked_publish( const std::string& client_id,
                                                                std::shared_ptr<protocol::puback> puback )
        {
            logger_->debug( "RX ACK: [cltid:{}|pkt:{}]", client_id, *puback );
            if ( const auto session = sessions_[client_id] )
            {
                session->client_acked_publish( puback );
            }
        }

        void mqtt_client_session_manager::client_received_publish( const std::string& client_id,
                                                                   std::shared_ptr<protocol::pubrec> pubrec )
        {
            logger_->debug( "RX REC: [cltid:{}|pkt:{}]", client_id, *pubrec );
            if ( const auto session = sessions_[client_id] )
            {
                session->client_received_publish( pubrec );
            }
        }

        void mqtt_client_session_manager::client_released_publish( const std::string& client_id,
                                                                   std::shared_ptr<protocol::pubrel> pubrel )
        {
            logger_->debug( "RX REL: [cltid:{}|pkt:{}]", client_id, *pubrel );
            if ( const auto session = sessions_[client_id] )
            {
                session->client_released_publish( pubrel );
            }
        }

        void mqtt_client_session_manager::client_completed_publish( const std::string& client_id,
                                                                    std::shared_ptr<protocol::pubcomp> pubcomp )
        {
            logger_->debug( "RX COMP: [cltid:{}|pkt:{}]", client_id, *pubcomp );
            if ( const auto session = sessions_[client_id] )
            {
                session->client_completed_publish( pubcomp );
            }
        }

        void mqtt_client_session_manager::client_disconnected_ungracefully( const std::string& client_id,
                                                                            dispatch::disconnect_reason reason )
        {
            sessions_.remove( client_id );
            logger_->info( "Client session [cltid:{}] destroyed after ungraceful disconnect: {}", client_id, reason );
        }

        std::size_t mqtt_client_session_manager::connected_clients_count( ) const
        {
            return sessions_.size( );
        }

        void mqtt_client_session_manager::destroy( const std::string& client_id )
        {
            sessions_.remove( client_id );
            logger_->info( "Client session [cltid:{}] destroyed", client_id );
        }

        void mqtt_client_session_manager::destroy_all( )
        {
            const auto sess_cnt = sessions_.size( );
            sessions_.clear( );
            logger_->info( "SHUTDOWN: [{}] client session(s) destroyed", sess_cnt );
        }

        // ------------------------------------------------------------------------------------------------------------
        // Private
        // ------------------------------------------------------------------------------------------------------------

        void mqtt_client_session_manager::publish( std::shared_ptr<protocol::publish> incoming_publish )
        {
            for ( const auto& subscriber : topic_subscriptions_.resolve_subscribers( incoming_publish ) )
            {
                if ( const auto session = sessions_[subscriber.first] )
                {
                    session->publish( incoming_publish, subscriber.second );
                }
            }
        }
    }  // namespace dispatch
}  // namespace io_wally
