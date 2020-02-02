#pragma once

#include <cstdint>
#include <sstream>

#include <cxxopts.hpp>

#include "io_wally/app/options_factory.hpp"
#include "io_wally/logging/logging.hpp"
#include "io_wally/spi/authentication_service_factory.hpp"

namespace io_wally
{
    struct context final
    {
       public:
        static constexpr const char* HELP = app::options_factory::HELP;

        static constexpr const char* LOG_FILE = app::options_factory::LOG_FILE;

        static constexpr const char* LOG_CONSOLE = app::options_factory::LOG_CONSOLE;

        static constexpr const char* LOG_LEVEL = app::options_factory::LOG_LEVEL;

        static constexpr const char* LOG_DISABLE = app::options_factory::LOG_DISABLE;

        static constexpr const char* SERVER_ADDRESS = app::options_factory::SERVER_ADDRESS;

        static constexpr const char* SERVER_PORT = app::options_factory::SERVER_PORT;

        static constexpr const char* AUTHENTICATION_SERVICE_FACTORY =
            app::options_factory::AUTHENTICATION_SERVICE_FACTORY;

        static constexpr const char* CONNECT_TIMEOUT = app::options_factory::CONNECT_TIMEOUT;

        static constexpr const char* READ_BUFFER_SIZE = app::options_factory::READ_BUFFER_SIZE;

        static constexpr const char* WRITE_BUFFER_SIZE = app::options_factory::WRITE_BUFFER_SIZE;

        static constexpr const char* PUB_ACK_TIMEOUT = app::options_factory::PUB_ACK_TIMEOUT;

        static constexpr const char* PUB_MAX_RETRIES = app::options_factory::PUB_MAX_RETRIES;

       public:
        context( cxxopts::ParseResult options,
                 std::unique_ptr<spi::authentication_service> authentication_service,
                 logging::logger_factory logger_factory )
            : options_{std::move( options )},
              authentication_service_{std::move( authentication_service )},
              logger_factory_{std::move( logger_factory )}
        {
            return;
        }

        context( context&& other ) noexcept
            : options_{other.options_},
              authentication_service_{std::move( other.authentication_service_ )},
              logger_factory_{other.logger_factory_}
        {
            return;
        }

        auto operator[]( const std::string& name ) const -> const cxxopts::OptionValue&
        {
            return options_[name];
        }

        auto authentication_service( ) const -> spi::authentication_service&
        {
            return *authentication_service_;
        }

        auto logger_factory( ) const -> const logging::logger_factory&
        {
            return logger_factory_;
        }

       private:
        const cxxopts::ParseResult options_;
        std::unique_ptr<spi::authentication_service> authentication_service_;
        const logging::logger_factory logger_factory_;
    };
}  // namespace io_wally
