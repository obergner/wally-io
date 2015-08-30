#pragma once

#include <boost/asio.hpp>

#include "io_wally/logging.hpp"
#include "io_wally/mqtt_session.hpp"
#include "io_wally/mqtt_session_manager.hpp"
#include "io_wally/spi/mqtt_packet_handler_factory.hpp"

using boost::asio::ip::tcp;

using namespace io_wally::spi;

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
    class mqtt_server
    {
       public:
        /// \brief \c mqtt_server instances cannot be copied
        mqtt_server( const mqtt_server& ) = delete;

        /// \brief \c mqtt_server instances cannot be copied
        mqtt_server& operator=( const mqtt_server& ) = delete;

        /// Construct the mqtt_server to listen on the specified TCP address and port.
        explicit mqtt_server( const std::string& address,
                              const std::string& port,
                              const mqtt_packet_handler_factory& packet_handler_factory );

        /// Run the mqtt_server's io_service loop.
        void run( );

       private:
        /// Perform an asynchronous accept operation.
        void do_accept( );

        /// Wait for a request to stop the mqtt_server.
        void do_await_stop( );

        /// Our session manager that manages all connections
        mqtt_session_manager session_manager_;

        /// Packet handler factory to be passed to each session
        const mqtt_packet_handler_factory& packet_handler_factory_;

        /// The io_service used to perform asynchronous operations.
        boost::asio::io_service io_service_;

        /// The signal_set is used to register for process termination notifications
        boost::asio::signal_set signals_;

        /// Acceptor used to listen for incoming connections.
        boost::asio::ip::tcp::acceptor acceptor_;

        /// The next socket to be accepted.
        boost::asio::ip::tcp::socket socket_;

        /// Our severity-enabled channel logger
        boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_;
    };
}  // namespace io_wally
