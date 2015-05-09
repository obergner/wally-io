#pragma once

#define BOOST_ALL_DYN_LINK 1

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>

#include <boost/asio.hpp>

namespace keywords = boost::log::keywords;
namespace lvl = boost::log::trivial;

namespace io_wally
{
    /// \brief All things logging in WallyIO.
    ///
    /// WallyIO uses Boost.Log v2 for all its logging.
    namespace logging
    {
        /// \brief Initialise logging subsystem.
        ///
        /// From \c log_file, initialise a Boost.Log file sink. If \c enable_console_log is \c true, also enable
        /// console logging.
        ///
        /// \param log_file     Path of log file to write logs into
        /// \param enable_console_log   Whether to log to console in addition to log file
        void init_logging( const std::string& log_file, bool enable_console_log );
    }
}

namespace boost
{
    /// \brief Extend namespace \c boost::asio with logging utility functions
    ///
    /// Implement \c operator<< for
    ///
    ///  - \c boost::asio::ip::tcp::endpoint
    ///  - \c boost::asio::ip::tcp::socket
    ///  - \c boost::asio::ip::tcp::acceptor
    ///
    namespace asio
    {
        /// \brief Overload stream output operator for boost::asio::ip::tcp::endpoint.
        ///
        /// Overload stream output operator for boost::asio::ip::tcp::endpoint, primarily to facilitate logging.
        inline std::ostream& operator<<( std::ostream& output, boost::asio::ip::tcp::endpoint const& endpoint )
        {
            output << "[addr:" << endpoint.address( ).to_string( ) << "|port:" << endpoint.port( ) << "]";

            return output;
        }

        /// \brief Overload stream output operator for boost::asio::ip::tcp::socket.
        ///
        /// Overload stream output operator for boost::asio::ip::tcp::socket, primarily to facilitate logging.
        inline std::ostream& operator<<( std::ostream& output, boost::asio::ip::tcp::socket const& socket )
        {
            output << "local:" << socket.local_endpoint( ) << "/remote:" << socket.remote_endpoint( );

            return output;
        }

        /// \brief Overload stream output operator for boost::asio::ip::tcp::acceptor.
        ///
        /// Overload stream output operator for boost::asio::ip::tcp::acceptor, primarily to facilitate logging.
        inline std::ostream& operator<<( std::ostream& output, boost::asio::ip::tcp::acceptor const& acceptor )
        {
            output << "accept:" << acceptor.local_endpoint( );

            return output;
        }
    }
}
