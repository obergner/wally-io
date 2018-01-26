#pragma once

#include <set>

#include <spdlog/spdlog.h>

#include "io_wally/context.hpp"
#include "io_wally/logging/logging.hpp"
#include "io_wally/logging_support.hpp"
#include "io_wally/mqtt_connection.hpp"

namespace io_wally
{
    /// Manages open \c mqtt_connections so that they may be cleanly stopped when the server
    /// needs to shut down.
    ///
    /// Rather unabashed copy:
    /// \see http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/example/cpp11/http/server/connection_manager.hpp
    class mqtt_connection_manager final
    {
       public:
        /// An mqtt_connection_manager cannot be copied.
        mqtt_connection_manager( const mqtt_connection_manager& ) = delete;
        /// An mqtt_connection_manager cannot be copied.
        mqtt_connection_manager& operator=( const mqtt_connection_manager& ) = delete;

        /// Construct a connection manager.
        mqtt_connection_manager( const context& context );

        /// Add the specified \c mqtt_connection to the manager and start it.
        void start( mqtt_connection::ptr connection );

        /// Stop the specified \c mqtt_connection.
        void stop( mqtt_connection::ptr connection );

        /// Stop all \c mqtt_connections.
        void stop_all( );

       private:
        /// The managed connections.
        std::set<mqtt_connection::ptr> connections_{};
        /// Our logger
        std::unique_ptr<spdlog::logger> logger_;
    };
}  // namespace io_wally
