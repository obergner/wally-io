#pragma once

#include <cstdint>
#include <sstream>

#include <boost/program_options.hpp>

#include "io_wally/spi/authentication_service_factory.hpp"

using namespace std;
namespace options = boost::program_options;

namespace io_wally
{
    struct context
    {
       public:
        static constexpr const char* HELP = "help";

        static constexpr const char* CONFIG_FILE = "conf-file";

        static constexpr const char* LOG_FILE = "log-file";

        static constexpr const char* LOG_CONSOLE = "log-console";

        static constexpr const char* LOG_SYNC = "log-sync";

        static constexpr const char* SERVER_ADDRESS = "server-address";

        static constexpr const char* SERVER_PORT = "server-port";

        static constexpr const char* AUTHENTICATION_SERVICE_FACTORY = "authentication-service-factory";

        static constexpr const char* CONNECT_TIMEOUT = "connect-timeout";

        static constexpr const char* READ_BUFFER_SIZE = "read-buffer-size";

        static constexpr const char* WRITE_BUFFER_SIZE = "write-buffer-size";

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
