#pragma once

#include <set>

#include "io_wally/mqtt_session.hpp"

namespace io_wally
{
    /// Manages open \c mqtt_sessions so that they may be cleanly stopped when the server
    /// needs to shut down.
    ///
    /// Rather unabashed copy:
    /// \see http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/example/cpp11/http/server/connection_manager.hpp
    class mqtt_session_manager
    {
       public:
        /// An mqtt_session_manager cannot be copied.
        mqtt_session_manager( const mqtt_session_manager& ) = delete;
        /// An mqtt_session_manager cannot be copied.
        mqtt_session_manager& operator=( const mqtt_session_manager& ) = delete;

        /// Construct a session manager.
        mqtt_session_manager( );

        /// Add the specified \c mqtt_session to the manager and start it.
        void start( mqtt_session::pointer session );

        /// Stop the specified \c mqtt_session.
        void stop( mqtt_session::pointer session );

        /// Stop all \c mqtt_sessions.
        void stop_all( );

       private:
        /// The managed sessions.
        std::set<mqtt_session::pointer> sessions_ = std::set<mqtt_session::pointer>{};

        /// Our severity-enabled channel logger
        boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_ =
            boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level>{
                keywords::channel = "session-manager",
                keywords::severity = lvl::trace};
    };
}  // namespace io_wally
