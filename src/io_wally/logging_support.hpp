#pragma once

#include <ostream>

#include <boost/asio.hpp>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

namespace io_wally
{
    namespace keywords = boost::log::keywords;
    namespace lvl = boost::log::trivial;
}  // namespace io_wally

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
