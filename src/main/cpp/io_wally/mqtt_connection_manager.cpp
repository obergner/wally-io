#include "io_wally/mqtt_connection_manager.hpp"

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include "io_wally/logging_support.hpp"

namespace io_wally
{
    mqtt_connection_manager::mqtt_connection_manager( )
    {
        return;
    }

    void mqtt_connection_manager::start( mqtt_connection::ptr connection )
    {
        connections_.insert( connection );
        connection->start( );
        logger_->debug( "STARTED: {}", *connection );
    }

    void mqtt_connection_manager::stop( mqtt_connection::ptr connection )
    {
        connections_.erase( connection );
        connection->do_stop( );
        logger_->debug( "STOPPED: {}", *connection );
    }

    void mqtt_connection_manager::stop_all( )
    {
        for ( auto& c : connections_ )
            c->do_stop( );
        connections_.clear( );
        logger_->debug( "All connections stopped" );
    }
}  // namespace io_wally
