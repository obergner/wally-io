#pragma once

#include <set>

#include "io_wally/mqtt_connection.hpp"

namespace io_wally
{
    /// Manages open \c mqtt_connections so that they may be cleanly stopped when the server
    /// needs to shut down.
    ///
    /// Rather unabashed copy:
    /// \see http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/example/cpp11/http/server/connection_manager.hpp
    class mqtt_connection_manager
    {
       public:
        /// An mqtt_connection_manager cannot be copied.
        mqtt_connection_manager( const mqtt_connection_manager& ) = delete;
        /// An mqtt_connection_manager cannot be copied.
        mqtt_connection_manager& operator=( const mqtt_connection_manager& ) = delete;

        /// Construct a session manager.
        mqtt_connection_manager( );

        /// Add the specified \c mqtt_connection to the manager and start it.
        void start( mqtt_connection::pointer session );

        /// Stop the specified \c mqtt_connection.
        void stop( mqtt_connection::pointer session );

        /// Stop all \c mqtt_connections.
        void stop_all( );

       private:
        /// The managed sessions.
        std::set<mqtt_connection::pointer> sessions_ = std::set<mqtt_connection::pointer>{};

        /// Our severity-enabled channel logger
        boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_ =
            boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level>{
                keywords::channel = "session-manager",
                keywords::severity = lvl::trace};
    };
}  // namespace io_wally
