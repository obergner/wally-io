#pragma once

#include <cstdint>
#include <map>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/logging_support.hpp"
#include "io_wally/mqtt_connection.hpp"
#include "io_wally/mqtt_client_session.hpp"

namespace io_wally
{
    /// Manages open \c mqtt_client_sessions so that they may be cleanly stopped when the server
    /// needs to shut down.
    ///
    /// WARNING: This class is NOT thread safe.
    class mqtt_client_session_manager
    {
       public:
        /// An mqtt_client_session_manager cannot be copied.
        mqtt_client_session_manager( const mqtt_client_session_manager& ) = delete;
        /// An mqtt_client_session_manager cannot be copied.
        mqtt_client_session_manager& operator=( const mqtt_client_session_manager& ) = delete;

        /// Construct a session manager.
        mqtt_client_session_manager( );

        void client_connected( const uint16_t client_id, mqtt_connection::ptr connection );

        /// Stop the specified \c mqtt_client_session.
        void stop( mqtt_client_session::ptr session );

        /// Stop all \c mqtt_client_sessions.
        void stop_all( );

       private:
        /// The managed sessions.
        std::map<uint16_t, mqtt_client_session::ptr> sessions_{};

        /// Our severity-enabled channel logger
        boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_{
            keywords::channel = "client-session-manager",
            keywords::severity = lvl::trace};
    };
}  // namespace io_wally
