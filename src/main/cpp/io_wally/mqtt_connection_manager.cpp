#include "io_wally/mqtt_connection_manager.hpp"

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/logging_support.hpp"

namespace io_wally
{
    namespace lvl = boost::log::trivial;

    mqtt_connection_manager::mqtt_connection_manager( )
    {
        return;
    }

    void mqtt_connection_manager::start( mqtt_connection::ptr connection )
    {
        connections_.insert( connection );
        connection->start( );
        BOOST_LOG_SEV( logger_, lvl::debug ) << "Started: " << *connection;
    }

    void mqtt_connection_manager::stop( mqtt_connection::ptr connection )
    {
        connections_.erase( connection );
        connection->do_stop( );
        BOOST_LOG_SEV( logger_, lvl::debug ) << "Stopped: " << *connection;
    }

    void mqtt_connection_manager::stop_all( )
    {
        for ( auto& c : connections_ )
            c->do_stop( );
        connections_.clear( );
        BOOST_LOG_SEV( logger_, lvl::debug ) << "All connections stopped";
    }
}  // namespace io_wally
