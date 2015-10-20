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

    void mqtt_connection_manager::start( mqtt_connection::ptr session )
    {
        sessions_.insert( session );
        session->start( );
        BOOST_LOG_SEV( logger_, lvl::debug ) << "Started: " << session;
    }

    void mqtt_connection_manager::stop( mqtt_connection::ptr session )
    {
        sessions_.erase( session );
        session->do_stop( );
        BOOST_LOG_SEV( logger_, lvl::debug ) << "Stopped: " << session;
    }

    void mqtt_connection_manager::stop_all( )
    {
        for ( auto c : sessions_ )
            c->do_stop( );
        sessions_.clear( );
        BOOST_LOG_SEV( logger_, lvl::debug ) << "All sessions stopped";
    }
}  // namespace io_wally
