#pragma once

#include <string>
#include <memory>
#include <map>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/logging_support.hpp"
#include "io_wally/mqtt_connection.hpp"
#include "io_wally/dispatch/common.hpp"
#include "io_wally/dispatch/mqtt_client_session.hpp"

namespace io_wally
{
    namespace dispatch
    {
        /// Manages open \c mqtt_client_sessions so that they may be cleanly stopped when the server
        /// needs to shut down.
        ///
        /// WARNING: This class is NOT thread safe.
        class mqtt_client_session_manager
        {
           public:
            /// \brief Create a session manager.
            mqtt_client_session_manager( ) = default;

            /// \brief Destroy this session manager, taking care to destroy all \c mqtt_client_session instances.
            ~mqtt_client_session_manager( );

            /// An mqtt_client_session_manager cannot be copied.
            mqtt_client_session_manager( const mqtt_client_session_manager& ) = delete;
            /// An mqtt_client_session_manager cannot be copied.
            mqtt_client_session_manager& operator=( const mqtt_client_session_manager& ) = delete;

            /// \brief Called when a new successful CONNECT request has been received. Creates a new \c
            /// mqtt_client_session.
            ///
            /// \param client_id   Client ID contained in CONNECT request. Uniquely identifies \c mqtt_client_session to
            ///                    be created.
            /// \param connection  \c mqtt_connection CONNECT request has been received on. Will be associated with new
            ///                    \c mqtt_client_session. Passed as a \c std::weak_ptr since \c mqtt_connection
            ///                    instances are owned by the network subsystem which may decide - potentially because
            ///                    network errors - to discard a connection at any time.
            void client_connected( const std::string& client_id, std::weak_ptr<mqtt_connection> connection );

            /// \brief Called when a client disconnects, either voluntarily by sending a DISCONNECT, or involuntarily
            /// due to network or protocol error. Destroys associated \c mqtt_client_session.
            ///
            /// \param client_id   Disconnected client's ID.
            /// \param reason      Why client was disconnected
            void client_disconnected( const std::string client_id, const dispatch::disconnect_reason reason );

            /// \brief Destroy the \c mqtt_client_session identified by specified \c client_id.
            ///
            /// \param client_id Client ID associated with \c mqtt_client_session to destroy (remove from this manager)
            void destroy( const std::string client_id );

            /// \brief Destroy all \c mqtt_client_sessions.
            void destroy_all( );

           private:
            /// The managed sessions.
            std::map<const std::string, mqtt_client_session::ptr> sessions_{};
            /// Our severity-enabled channel logger
            boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_{
                boost::log::keywords::channel = "client-session-manager",
                boost::log::keywords::severity = boost::log::trivial::trace};
        };
    }  // namespace dispatch
}  // namespace io_wally
