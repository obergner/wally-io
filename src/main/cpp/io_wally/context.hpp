#pragma once

#include <cstdint>
#include <sstream>

#include <boost/program_options.hpp>

#include "io_wally/app/options_parser.hpp"
#include "io_wally/spi/authentication_service_factory.hpp"

namespace io_wally
{
    struct context final
    {
       public:
        static constexpr const char* HELP = app::options_parser::HELP;

        static constexpr const char* CONFIG_FILE = app::options_parser::CONFIG_FILE;

        static constexpr const char* LOG_FILE = app::options_parser::LOG_FILE;

        static constexpr const char* LOG_FILE_LEVEL = app::options_parser::LOG_FILE_LEVEL;

        static constexpr const char* LOG_CONSOLE = app::options_parser::LOG_CONSOLE;

        static constexpr const char* LOG_CONSOLE_LEVEL = app::options_parser::LOG_CONSOLE_LEVEL;

        static constexpr const char* LOG_SYNC = app::options_parser::LOG_SYNC;

        static constexpr const char* LOG_DISABLE = app::options_parser::LOG_DISABLE;

        static constexpr const char* SERVER_ADDRESS = app::options_parser::SERVER_ADDRESS;

        static constexpr const char* SERVER_PORT = app::options_parser::SERVER_PORT;

        static constexpr const char* AUTHENTICATION_SERVICE_FACTORY =
            app::options_parser::AUTHENTICATION_SERVICE_FACTORY;

        static constexpr const char* CONNECT_TIMEOUT = app::options_parser::CONNECT_TIMEOUT;

        static constexpr const char* READ_BUFFER_SIZE = app::options_parser::READ_BUFFER_SIZE;

        static constexpr const char* WRITE_BUFFER_SIZE = app::options_parser::WRITE_BUFFER_SIZE;

       public:
        context( boost::program_options::variables_map options,
                 std::unique_ptr<spi::authentication_service> authentication_service )
            : options_{move( options )}, authentication_service_{std::move( authentication_service )}
        {
            return;
        }

        context( context&& other )
            : options_{move( other.options_ )}, authentication_service_{std::move( other.authentication_service_ )}
        {
            return;
        }

        const boost::program_options::variable_value& operator[]( const std::string& name ) const
        {
            return options_[name];
        }

        spi::authentication_service& authentication_service( ) const
        {
            return *authentication_service_;
        }

       private:
        const boost::program_options::variables_map options_;
        std::unique_ptr<spi::authentication_service> authentication_service_;
    };
}