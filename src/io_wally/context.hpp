#pragma once

#include <cstdint>
#include <sstream>

#include <boost/program_options.hpp>

#include "io_wally/app/options_parser.hpp"
#include "io_wally/spi/authentication_service_factory.hpp"

namespace io_wally
{
    using namespace std;
    namespace options = boost::program_options;

    struct context
    {
       public:
        static constexpr const char* HELP = io_wally::app::options_parser::HELP;

        static constexpr const char* CONFIG_FILE = io_wally::app::options_parser::CONFIG_FILE;

        static constexpr const char* LOG_FILE = io_wally::app::options_parser::LOG_FILE;

        static constexpr const char* LOG_CONSOLE = io_wally::app::options_parser::LOG_CONSOLE;

        static constexpr const char* LOG_SYNC = io_wally::app::options_parser::LOG_SYNC;

        static constexpr const char* SERVER_ADDRESS = io_wally::app::options_parser::SERVER_ADDRESS;

        static constexpr const char* SERVER_PORT = io_wally::app::options_parser::SERVER_PORT;

        static constexpr const char* AUTHENTICATION_SERVICE_FACTORY =
            io_wally::app::options_parser::AUTHENTICATION_SERVICE_FACTORY;

        static constexpr const char* CONNECT_TIMEOUT = io_wally::app::options_parser::CONNECT_TIMEOUT;

        static constexpr const char* READ_BUFFER_SIZE = io_wally::app::options_parser::READ_BUFFER_SIZE;

        static constexpr const char* WRITE_BUFFER_SIZE = io_wally::app::options_parser::WRITE_BUFFER_SIZE;

       public:
        context( const options::variables_map options,
                 unique_ptr<io_wally::spi::authentication_service> authentication_service )
            : options_{move( options )}, authentication_service_{move( authentication_service )}
        {
            return;
        }

        context( context&& other )
            : options_{move( other.options_ )}, authentication_service_{move( other.authentication_service_ )}
        {
            return;
        }

        const options::variables_map& options( ) const
        {
            return options_;
        }

        io_wally::spi::authentication_service& authentication_service( ) const
        {
            return *authentication_service_;
        }

       private:
        const options::variables_map options_;
        unique_ptr<io_wally::spi::authentication_service> authentication_service_;
    };
}
