#include "io_wally/mqtt_connection_manager.hpp"

namespace io_wally
{
    mqtt_connection_manager::mqtt_connection_manager( )
    {
        return;
    }

    void mqtt_connection_manager::start( mqtt_connection::pointer session )
    {
        sessions_.insert( session );
        session->start( );
        BOOST_LOG_SEV( logger_, lvl::debug ) << "Started: " << session;
    }

    void mqtt_connection_manager::stop( mqtt_connection::pointer session )
    {
        sessions_.erase( session );
        session->stop( );
        BOOST_LOG_SEV( logger_, lvl::debug ) << "Stopped: " << session;
    }

    void mqtt_connection_manager::stop_all( )
    {
        for ( auto c : sessions_ )
            c->stop( );
        sessions_.clear( );
        BOOST_LOG_SEV( logger_, lvl::debug ) << "All sessions stopped";
    }
}  // namespace io_wally
