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

namespace io_wally
{
    namespace logging
    {
        namespace lvl
        {
            enum severity_level : int
            {
                trace = 0,
                debug,
                info,
                warn,
                error,
                fatal
            };
        }

        //! Returns stringized enumeration value or \c NULL, if the value is not valid
        const char* to_string( lvl::severity_level lvl );

        //! Outputs stringized representation of the severity level to the stream
        template <typename CharT, typename TraitsT>
        inline std::basic_ostream<CharT, TraitsT>& operator<<( std::basic_ostream<CharT, TraitsT>& strm,
                                                               lvl::severity_level lvl )
        {
            const char* str = to_string( lvl );
            if ( str )
                strm << str;
            else
                strm << static_cast<int>( lvl );
            return strm;
        }

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
