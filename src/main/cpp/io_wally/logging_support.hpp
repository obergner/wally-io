#pragma once

#include <ostream>

#include <asio.hpp>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

/// \brief Extend namespace \c asio with logging utility functions
///
/// Implement \c operator<< for
///
///  - \c ::asio::ip::tcp::endpoint
///  - \c ::asio::ip::tcp::socket
///  - \c ::asio::ip::tcp::acceptor
///
namespace asio
{
    /// \brief Overload stream output operator for ::asio::ip::tcp::endpoint.
    ///
    /// Overload stream output operator for ::asio::ip::tcp::endpoint, primarily to facilitate logging.
    inline std::ostream& operator<<( std::ostream& output, ::asio::ip::tcp::endpoint const& endpoint )
    {
        output << "[addr:" << endpoint.address( ).to_string( ) << "|port:" << endpoint.port( ) << "]";

        return output;
    }

    /// \brief Overload stream output operator for ::asio::ip::tcp::socket.
    ///
    /// Overload stream output operator for ::asio::ip::tcp::socket, primarily to facilitate logging.
    inline std::ostream& operator<<( std::ostream& output, ::asio::ip::tcp::socket const& socket )
    {
        // WARNING: Calling to_string() on a closed socket will crash process!
        if ( socket.is_open( ) )
            output << "local:" << socket.local_endpoint( ) << "/remote:" << socket.remote_endpoint( );
        else
            output << "[CLOSED SOCKET]";

        return output;
    }

    /// \brief Overload stream output operator for ::asio::ip::tcp::acceptor.
    ///
    /// Overload stream output operator for ::asio::ip::tcp::acceptor, primarily to facilitate logging.
    inline std::ostream& operator<<( std::ostream& output, ::asio::ip::tcp::acceptor const& acceptor )
    {
        // WARNING: Calling to_string() on a closed acceptor will crash process!
        if ( acceptor.is_open( ) )
            output << "accept:" << acceptor.local_endpoint( );
        else
            output << "[CLOSED ACCEPTOR]";

        return output;
    }
}
