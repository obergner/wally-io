#pragma once

#include <memory>

#include <signal.h>

#include <boost/asio.hpp>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/logging_support.hpp"
#include "io_wally/context.hpp"
#include "io_wally/mqtt_connection.hpp"
#include "io_wally/mqtt_connection_manager.hpp"
#include "io_wally/spi/authentication_service_factory.hpp"

namespace io_wally
{
    /// \brief The MQTT server.
    ///
    /// An \c mqtt_server instance is initialized with an \c address and a \c port to listen on. It starts
    /// listening for incoming connection requests as soon as a client calls its run() method.
    ///
    /// To stop a running \c mqtt_server instance users currently need to send a termination signal to the running
    /// process.
    ///
    /// \todo Provide an explicit stop method.
    ///
    /// \see http://www.boost.org/doc/libs/1_58_0/doc/html/boost_asio/example/cpp11/http/server/server.hpp
    class mqtt_server : public std::enable_shared_from_this<mqtt_server>
    {
       public:
        /// A \c shared_ptr to an \c mqtt_server.
        typedef shared_ptr<mqtt_server> pointer;

        /// Factory method for \c mqtt_connections.
        static pointer create( context );

        /// \brief \c mqtt_server instances cannot be copied
        mqtt_server( const mqtt_server& ) = delete;

        /// \brief \c mqtt_server instances cannot be copied
        mqtt_server& operator=( const mqtt_server& ) = delete;

        /// Run the mqtt_server's io_service loop.
        void run( );

        /// Shut down this server, closing all client connections
        void shutdown( const std::string message = "" );

       private:
        /// Construct the mqtt_server to listen on the specified TCP address and port.
        explicit mqtt_server( context context );

        /// Perform an asynchronous accept operation.
        void do_accept( );

        /// Wait for a request to stop the mqtt_server.
        void do_await_stop( );

        /// Internal shutdown method, delegated to by \c shutdown().
        void do_shutdown( const std::string& message = "" );

       private:
        /// Context object
        const context context_;

        /// Our session manager that manages all connections
        mqtt_connection_manager connection_manager_{};

        /// The io_service used to perform asynchronous operations.
        boost::asio::io_service io_service_{};

        /// The signal_set is used to register for process termination notifications
        boost::asio::signal_set termination_signals_{io_service_, SIGINT, SIGTERM, SIGQUIT};

        /// Acceptor used to listen for incoming connections.
        boost::asio::ip::tcp::acceptor acceptor_{io_service_};

        /// The next socket to be accepted.
        boost::asio::ip::tcp::socket socket_{io_service_};

        /// Our severity-enabled channel logger
        boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_{
            keywords::channel = "server",
            keywords::severity = lvl::trace};
    };
}  // namespace io_wally
