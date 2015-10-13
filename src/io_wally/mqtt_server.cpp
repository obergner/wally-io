#include "io_wally/mqtt_server.hpp"

#include <mutex>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/logging_support.hpp"

namespace io_wally
{
    using namespace std;

    mqtt_server::pointer mqtt_server::create( context context )
    {
        return pointer( new mqtt_server( move( context ) ) );
    }

    mqtt_server::mqtt_server( io_wally::context context ) : context_{move( context )}
    {
        return;
    }

    void mqtt_server::run( )
    {
        BOOST_LOG_SEV( logger_, lvl::info ) << "START: MQTT server ...";

        do_await_stop( );

        const string& address = context_.options( )[io_wally::context::SERVER_ADDRESS].as<const string>( );
        const int port = context_.options( )[io_wally::context::SERVER_PORT].as<const int>( );
        boost::asio::ip::tcp::resolver resolver( io_service_ );
        const boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve( {address, to_string( port )} );

        acceptor_.open( endpoint.protocol( ) );
        // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
        acceptor_.set_option( boost::asio::ip::tcp::acceptor::reuse_address( true ) );
        acceptor_.bind( endpoint );
        acceptor_.listen( );

        do_accept( );
        {
            // Use nested scope to guaratuee that lock is released
            unique_lock<mutex> ul{bind_mutex_};
            bound_.notify_all( );
        }

        BOOST_LOG_SEV( logger_, lvl::info ) << "STARTED: MQTT server (" << acceptor_ << ")";

        // The io_service::run() call will block until all asynchronous operations
        // have finished. While the mqtt_server is running, there is always at least one
        // asynchronous operation outstanding: the asynchronous accept call waiting
        // for new incoming connections.
        io_service_.run( );
    }

    void mqtt_server::wait_for_bound( )
    {
        unique_lock<mutex> ul{bind_mutex_};
        bound_.wait( ul,
                     [this]( )
                     {
            return acceptor_.is_open( );
        } );
    }

    void mqtt_server::do_accept( )
    {
        auto self( shared_from_this( ) );
        acceptor_.async_accept( socket_,
                                [self]( const boost::system::error_code& ec )
                                {
            BOOST_LOG_SEV( self->logger_, lvl::debug ) << "ACCEPTED: " << self->socket_;
            // Check whether the mqtt_server was stopped by a signal before this
            // completion handler had a chance to run.
            if ( !self->acceptor_.is_open( ) )
            {
                return;
            }
            if ( !ec )
            {
                mqtt_connection::pointer session =
                    mqtt_connection::create( move( self->socket_ ), self->connection_manager_, self->context_ );
                self->connection_manager_.start( session );
            }

            self->do_accept( );
        } );
    }

    void mqtt_server::do_await_stop( )
    {
        // See: http://www.boost.org/doc/libs/1_59_0/doc/html/boost_asio/reference/basic_signal_set/async_wait.html
        auto self( shared_from_this( ) );
        termination_signals_.async_wait( [self]( const boost::system::error_code& ec, int signo )
                                         {
                                             // Signal set was cancelled. Should not happen since termination_signals_
                                             // is private to this class and we sure don't want to cancel it, by golly!
                                             assert( !ec );

                                             self->do_shutdown( "Received termination signal [" + to_string( signo ) +
                                                                "] - MQTT server will stop ..." );
                                         } );
    }

    void mqtt_server::shutdown( const std::string message )
    {
        auto self( shared_from_this( ) );
        io_service_.post( [self, message]( )
                          {
                              self->do_shutdown( message );
                          } );
    }

    void mqtt_server::do_shutdown( const std::string& message )
    {
        BOOST_LOG_SEV( logger_, lvl::debug ) << message;
        // The mqtt_server is stopped by cancelling all outstanding asynchronous operations.Once all operations have
        // finished the io_service::run( ) call will exit.
        if ( acceptor_.is_open( ) )
            acceptor_.close( );
        connection_manager_.stop_all( );
        if ( !io_service_.stopped( ) )
            io_service_.stop( );

        BOOST_LOG_SEV( logger_, lvl::info ) << "STOPPED: MQTT server";
    }

}  // namespace io_wally
