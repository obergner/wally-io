#pragma once

#include <string>
#include <memory>
#include <map>

#include <asio.hpp>
#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/logging_support.hpp"
#include "io_wally/context.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/protocol/unsubscribe_packet.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/puback_packet.hpp"
#include "io_wally/protocol/pubrec_packet.hpp"
#include "io_wally/protocol/pubcomp_packet.hpp"
#include "io_wally/dispatch/common.hpp"
#include "io_wally/dispatch/mqtt_client_session.hpp"
#include "io_wally/dispatch/topic_subscriptions.hpp"
#include "io_wally/dispatch/retained_messages.hpp"

namespace io_wally
{
    namespace dispatch
    {
        /// Manages open \c mqtt_client_sessions so that they may be cleanly stopped when the server
        /// needs to shut down.
        ///
        /// WARNING: This class is NOT thread safe.
        class mqtt_client_session_manager final
        {
            friend class mqtt_client_session;

           public:
            /// \brief Create a session manager.
            mqtt_client_session_manager( const context& context, ::asio::io_service& io_service );

            /// \brief Destroy this session manager, taking care to destroy all \c mqtt_client_session instances.
            ~mqtt_client_session_manager( );

            /// An mqtt_client_session_manager cannot be copied.
            mqtt_client_session_manager( const mqtt_client_session_manager& ) = delete;
            /// An mqtt_client_session_manager cannot be copied.
            mqtt_client_session_manager& operator=( const mqtt_client_session_manager& ) = delete;

            /// \brief Return \c context associated with this session manager.
            ///
            /// \return \c context associated with this session manager
            const io_wally::context& context( ) const;

            /// \brief Return \c io_service associated with this session manager.
            ///
            /// \return \c io_service associated with this session manager
            ::asio::io_service& io_service( ) const;

            /// \brief Called when a new successful CONNECT request has been received. Creates a new \c
            /// mqtt_client_session.
            ///
            /// \param client_id   Client ID contained in CONNECT request. Uniquely identifies \c mqtt_client_session to
            ///                    be created.
            /// \param connection  \c mqtt_packet_sender CONNECT request has been received on. Will be associated with
            /// new
            ///                    \c mqtt_client_session. Passed as a \c std::weak_ptr since \c mqtt_packet_sender
            ///                    instances are owned by the network subsystem which may decide - potentially because
            ///                    network errors - to discard a connection at any time.
            void client_connected( const std::string& client_id, std::weak_ptr<mqtt_packet_sender> connection );

            /// \brief Called when a client disconnects, either voluntarily by sending a DISCONNECT, or involuntarily
            /// due to network or protocol error. Destroys associated \c mqtt_client_session.
            ///
            /// \param client_id   Disconnected client's ID.
            /// \param reason      Why client was disconnected
            void client_disconnected( const std::string& client_id, const dispatch::disconnect_reason reason );

            /// \brief Called when a client sent a SUBSCRIBE packet.
            ///
            /// \param client_id ID of client that sent SUBSCRIBE packet
            /// \param subscribe MQTT SUBSCRIBE packet received from client
            void client_subscribed( const std::string& client_id, std::shared_ptr<protocol::subscribe> subscribe );

            /// \brief Called when a client sent an UNSUBSCRIBE packet.
            ///
            /// \param client_id ID of client that sent UNSUBSCRIBE packet
            /// \param usubscribe MQTT UNSUBSCRIBE packet received from client
            void client_unsubscribed( const std::string& client_id,
                                      std::shared_ptr<protocol::unsubscribe> unsubscribe );

            /// \brief Called when client sent a PUBLISH packet.
            ///
            /// \param client_id ID of client that sent PUBLISH packet
            /// \param incoming_publish MQTT PUBLISH packet received from client \c client_id
            void client_published( const std::string& client_id, std::shared_ptr<protocol::publish> incoming_publish );

            /// \brief Called when a client acknowledged a received QoS 1 PUBLISH, i.e. sent a PUBACK
            ///
            /// \param client_id ID of client that acked PUBLISH
            /// \param puback PUBACK sent by connected client
            void client_acked_publish( const std::string& client_id, std::shared_ptr<protocol::puback> puback );

            /// \brief Called when a client received a QoS 2 PUBLISH, i.e. sent a PUBREC.
            ///
            /// \param client_id ID of client that received PUBLISH
            /// \param pubrec PUBREC sent by connected client
            void client_received_publish( const std::string& client_id, std::shared_ptr<protocol::pubrec> pubrec );

            /// \brief Called when a client released a QoS 2 PUBLISH, i.e. sent a PUBREL.
            ///
            /// \param client_id ID of client that received PUBLISH
            /// \param pubrel PUBREL sent by connected client
            void client_released_publish( const std::string& client_id, std::shared_ptr<protocol::pubrel> pubrel );

            /// \brief Called when a client completed a QoS 2 PUBLISH, i.e. sent a PUBCOMP.
            ///
            /// \param client_id ID of client that completed PUBLISH
            /// \param pubrec PUBCOMP sent by connected client
            void client_completed_publish( const std::string& client_id, std::shared_ptr<protocol::pubcomp> pubcomp );

            /// \brief Destroy the \c mqtt_client_session identified by specified \c client_id.
            ///
            /// \param client_id Client ID associated with \c mqtt_client_session to destroy (remove from this manager)
            void destroy( const std::string& client_id );

            /// \brief Destroy all \c mqtt_client_sessions.
            void destroy_all( );

           private:
            void publish( std::shared_ptr<protocol::publish> incoming_publish );

           private:
            /// Our configuration context, to be passed on to client sessions
            const io_wally::context& context_;
            /// Boost Asio io_service, to be passed on to client sessions
            ::asio::io_service& io_service_;
            /// Where we store topic subscriptions
            topic_subscriptions topic_subscriptions_;
            /// The managed sessions.
            std::map<const std::string, mqtt_client_session::ptr> sessions_{};
            /// All retained messages
            retained_messages retained_messages_{};
            /// Our severity-enabled channel logger
            boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_{
                boost::log::keywords::channel = "client-session-manager",
                boost::log::keywords::severity = boost::log::trivial::trace};
        };
    }  // namespace dispatch
}  // namespace io_wally
