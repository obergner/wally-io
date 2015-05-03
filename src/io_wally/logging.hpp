#pragma once

#define BOOST_ALL_DYN_LINK 1

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

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
        inline std::ostream& operator<<( std::ostream& output, boost::asio::ip::tcp::endpoint const& endpoint )
        {
            output << "[addr:" << endpoint.address( ).to_string( ) << "|port:" << endpoint.port( ) << "]";

            return output;
        }

        inline std::ostream& operator<<( std::ostream& output, boost::asio::ip::tcp::socket const& socket )
        {
            output << "local:" << socket.local_endpoint( ) << "/remote:" << socket.remote_endpoint( );

            return output;
        }

        inline std::ostream& operator<<( std::ostream& output, boost::asio::ip::tcp::acceptor const& acceptor )
        {
            output << "accept:" << acceptor.local_endpoint( );

            return output;
        }
    }
}
