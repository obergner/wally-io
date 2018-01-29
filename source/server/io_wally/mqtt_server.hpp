#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>

#include <signal.h>

#include <asio.hpp>

#include <spdlog/spdlog.h>

#include "io_wally/concurrency/io_service_pool.hpp"
#include "io_wally/context.hpp"
#include "io_wally/dispatch/dispatcher.hpp"
#include "io_wally/logging/logging.hpp"
#include "io_wally/logging_support.hpp"
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
    class mqtt_server final : public std::enable_shared_from_this<mqtt_server>
    {
       public:  // static
        /// A \c shared_ptr to an \c mqtt_server.
        using ptr = std::shared_ptr<mqtt_server>;

        /// Factory method for \c mqtt_servers.
        static mqtt_server::ptr create( context );

       public:
        /// \brief \c mqtt_server instances cannot be copied
        mqtt_server( const mqtt_server& ) = delete;

        /// \brief \c mqtt_server instances cannot be copied
        mqtt_server& operator=( const mqtt_server& ) = delete;

        /// Run the mqtt_server's io_service loop.
        void run( );

        /// Wait for this server to have bound to its server socket
        void wait_until_bound( );

        /// Request to close all client connections
        void close_connections( const std::string& message = "" );

        /// Wait for this server to have unbound from its server socket
        void wait_until_connections_closed( );

        /// Shut down this server, closing all client connections
        void stop( const std::string& message = "" );

        /// Wait for for this server to have stopped, i.e. its internal \c io_service_pool to have stopped
        void wait_until_stopped( );

       private:
        /// Construct the mqtt_server to listen on the specified TCP address and port.
        explicit mqtt_server( context context );

        /// Perform an asynchronous accept operation.
        void do_accept( );

        /// Wait for a shutdown signal, one of SIGINT, SIGTERM, SIGQUIT.
        void do_await_stop( );

        /// Internal unbind method
        void do_close_connections( const std::string& message = "" );

       private:
        /// Context object
        const context context_;
        std::mutex bind_mutex_{};
        /// Signal when we are bound to our server socket
        std::condition_variable bound_{};
        /// Signal when all client connections have been closed
        bool connections_closed_{false};
        std::condition_variable conn_closed_{};
        /// Our session manager that manages all connections
        mqtt_connection_manager connection_manager_{context_};
        /// Pool of io_service objects used for all things networking (just one io_service object for now)
        concurrency::io_service_pool network_service_pool_{context_, "network", 1};
        /// The io_service used to perform asynchronous operations.
        ::asio::io_service& io_service_{network_service_pool_.io_service( )};
        /// Dispatcher: dispatch received packets to dispatcher subsystem
        dispatch::dispatcher dispatcher_{context_, io_service_};
        /// The signal_set is used to register for process termination notifications
        ::asio::signal_set termination_signals_{io_service_, SIGINT, SIGTERM, SIGQUIT};
        /// Acceptor used to listen for incoming connections.
        ::asio::ip::tcp::acceptor acceptor_{io_service_};
        /// The next socket to be accepted.
        ::asio::ip::tcp::socket socket_{io_service_};
        /// Our logger
        std::unique_ptr<spdlog::logger> logger_ = context_.logger_factory( ).logger( "server" );
    };
}  // namespace io_wally
