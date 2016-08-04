#include "io_wally/mqtt_server.hpp"

#include <mutex>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/dispatch/common.hpp"
#include "io_wally/logging_support.hpp"

namespace io_wally
{
    using namespace std;
    namespace lvl = boost::log::trivial;

    // ---------------------------------------------------------------------------------------------------------------
    // Public
    // ---------------------------------------------------------------------------------------------------------------

    mqtt_server::ptr mqtt_server::create( context context, mqtt_connection::packetq_t& dispatchq )
    {
        return ptr( new mqtt_server( move( context ), dispatchq ) );
    }

    mqtt_server::mqtt_server( io_wally::context context, mqtt_connection::packetq_t& dispatchq )
        : context_{move( context )}, dispatchq_{dispatchq}
    {
        return;
    }

    void mqtt_server::run( )
    {
        BOOST_LOG_SEV( logger_, lvl::info ) << "START: MQTT server ...";

        do_await_stop( );

        auto address = context_[io_wally::context::SERVER_ADDRESS].as<const string>( );
        auto port = context_[io_wally::context::SERVER_PORT].as<const int>( );
        boost::asio::ip::tcp::resolver resolver{io_service_};
        const boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve( {address, to_string( port )} );

        acceptor_.open( endpoint.protocol( ) );
        // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
        acceptor_.set_option( boost::asio::ip::tcp::acceptor::reuse_address( true ) );
        acceptor_.bind( endpoint );
        acceptor_.listen( );

        do_accept( );

        network_service_pool_.run( );
        BOOST_LOG_SEV( logger_, lvl::info ) << "STARTED: MQTT server (" << acceptor_ << ")";

        {
            // Use nested scope to guaratuee that lock is released
            auto ul = unique_lock<mutex>{bind_mutex_};
            bound_.notify_all( );
        }
    }

    void mqtt_server::wait_until_bound( )
    {
        auto ul = unique_lock<mutex>{bind_mutex_};
        bound_.wait( ul, [this]( ) { return acceptor_.is_open( ); } );
    }

    void mqtt_server::close_connections( const std::string& message )
    {
        auto self = shared_from_this( );
        io_service_.post( [self, message]( ) { self->do_close_connections( message ); } );
    }

    void mqtt_server::wait_until_connections_closed( )
    {
        auto ul = unique_lock<mutex>{bind_mutex_};
        conn_closed_.wait( ul, [this]( ) { return connections_closed_; } );
    }

    void mqtt_server::stop( const std::string& message )
    {
        network_service_pool_.stop( );

        BOOST_LOG_SEV( logger_, lvl::debug ) << message;
    }

    void mqtt_server::wait_until_stopped( )
    {
        network_service_pool_.wait_until_stopped( );
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Private
    // ---------------------------------------------------------------------------------------------------------------

    void mqtt_server::do_accept( )
    {
        auto self = shared_from_this( );
        acceptor_.async_accept( socket_, [self]( const boost::system::error_code& ec ) {
            BOOST_LOG_SEV( self->logger_, lvl::debug ) << "ACCEPTED: " << self->socket_;
            // Check whether the mqtt_server was stopped by a signal before this
            // completion handler had a chance to run.
            if ( !self->acceptor_.is_open( ) )
            {
                return;
            }
            if ( !ec )
            {
                mqtt_connection::ptr session = mqtt_connection::create(
                    move( self->socket_ ), self->connection_manager_, self->context_, self->dispatcher_ );
                self->connection_manager_.start( session );
            }

            self->do_accept( );
        } );
    }

    void mqtt_server::do_await_stop( )
    {
        // See: http://www.boost.org/doc/libs/1_59_0/doc/html/boost_asio/reference/basic_signal_set/async_wait.html
        auto self = shared_from_this( );
        termination_signals_.async_wait( [self]( const boost::system::error_code& ec, int signo ) {
            // Signal set was cancelled. Should not happen since termination_signals_
            // is private to this class and we sure don't want to cancel it, by golly!
            assert( !ec );

            self->do_close_connections( "Received termination signal [" + to_string( signo ) +
                                        "] - MQTT server will close all client connections ..." );
        } );
    }

    void mqtt_server::do_close_connections( const std::string& message )
    {
        BOOST_LOG_SEV( logger_, lvl::debug ) << message;

        connection_manager_.stop_all( );

        {
            auto ul = unique_lock<mutex>{bind_mutex_};
            connections_closed_ = true;
            conn_closed_.notify_all( );
        }

        BOOST_LOG_SEV( logger_, lvl::info ) << "UNBOUND: MQTT server";
    }

}  // namespace io_wally
