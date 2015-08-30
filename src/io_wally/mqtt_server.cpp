#include <signal.h>
#include <utility>

#include "io_wally/logging.hpp"
#include "io_wally/mqtt_server.hpp"

using boost::asio::ip::tcp;
using namespace io_wally::logging;

namespace io_wally
{

    mqtt_server::mqtt_server( const std::string& address,
                              const std::string& port,
                              const mqtt_packet_handler_factory& packet_handler_factory )
        : session_manager_( ),
          packet_handler_factory_( packet_handler_factory ),
          io_service_( ),
          signals_( io_service_ ),
          acceptor_( io_service_ ),
          socket_( io_service_ ),
          logger_( keywords::channel = "server", keywords::severity = lvl::trace )
    {
        // Register to handle the signals that indicate when the mqtt_server should exit.
        // It is safe to register for the same signal multiple times in a program,
        // provided all registration for the specified signal is made through Asio.
        signals_.add( SIGINT );
        signals_.add( SIGTERM );
#if defined( SIGQUIT )
        signals_.add( SIGQUIT );
#endif  // defined(SIGQUIT)

        do_await_stop( );

        // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
        boost::asio::ip::tcp::resolver resolver( io_service_ );
        boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve( {address, port} );
        acceptor_.open( endpoint.protocol( ) );
        acceptor_.set_option( boost::asio::ip::tcp::acceptor::reuse_address( true ) );
        acceptor_.bind( endpoint );
        acceptor_.listen( );

        do_accept( );
    }

    void mqtt_server::run( )
    {
        BOOST_LOG_SEV( logger_, lvl::info ) << "START: MQTT server: " << acceptor_;
        // The io_service::run() call will block until all asynchronous operations
        // have finished. While the mqtt_server is running, there is always at least one
        // asynchronous operation outstanding: the asynchronous accept call waiting
        // for new incoming connections.
        io_service_.run( );
    }

    void mqtt_server::do_accept( )
    {
        acceptor_.async_accept( socket_, [this]( boost::system::error_code ec )
                                {
                                    BOOST_LOG_SEV( logger_, lvl::debug ) << "ACCEPTED: " << socket_;
                                    // Check whether the mqtt_server was stopped by a signal before this
                                    // completion handler had a chance to run.
                                    if ( !acceptor_.is_open( ) )
                                    {
                                        return;
                                    }
                                    if ( !ec )
                                    {
                                        mqtt_session::pointer session =
                                            mqtt_session::create( std::move( socket_ ), session_manager_ );
                                        session_manager_.start( session );
                                    }

                                    do_accept( );
                                } );
    }

    void mqtt_server::do_await_stop( )
    {
        signals_.async_wait( [this]( boost::system::error_code /* ec */, int /* signo*/ )
                             {
                                 // The mqtt_server is stopped by cancelling all outstanding asynchronous
                                 // operations. Once all operations have finished the io_service::run( ) call will exit.
                                 acceptor_.close( );
                                 session_manager_.stop_all( );
                                 BOOST_LOG_SEV( logger_, lvl::info ) << "STOPPED: MQTT server";
                             } );
    }

}  // namespace io_wally
