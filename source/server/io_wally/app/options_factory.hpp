#pragma once

#include <string>
#include <vector>

#include <cxxopts.hpp>

namespace io_wally::app
{
    /// \brief Utility class for creating a configured instance of \c cxxopts::Options
    class options_factory final
    {
       public:  // static
        static constexpr const char* HELP = "help";
        static constexpr const char* HELP_SPEC = "h,help";

        static constexpr const char* LOG_FILE = "log-file";
        static constexpr const char* LOG_FILE_SPEC = "f,log-file";

        static constexpr const char* LOG_CONSOLE = "log-console";
        static constexpr const char* LOG_CONSOLE_SPEC = "log-console";

        static constexpr const char* LOG_LEVEL = "log-level";
        static constexpr const char* LOG_LEVEL_SPEC = "log-level";

        static constexpr const char* LOG_DISABLE = "log-disable";
        static constexpr const char* LOG_DISABLE_SPEC = "s,log-disable";

        static constexpr const char* SERVER_ADDRESS = "server-address";
        static constexpr const char* SERVER_ADDRESS_SPEC = "l,server-address";

        static constexpr const char* SERVER_PORT = "server-port";
        static constexpr const char* SERVER_PORT_SPEC = "p,server-port";

        static constexpr const char* AUTHENTICATION_SERVICE_FACTORY = "auth-service-factory";
        static constexpr const char* AUTHENTICATION_SERVICE_FACTORY_SPEC = "auth-service-factory";

        static constexpr const char* CONNECT_TIMEOUT = "conn-timeout";
        static constexpr const char* CONNECT_TIMEOUT_SPEC = "t,conn-timeout";

        static constexpr const char* READ_BUFFER_SIZE = "conn-rbuf-size";
        static constexpr const char* READ_BUFFER_SIZE_SPEC = "conn-rbuf-size";

        static constexpr const char* WRITE_BUFFER_SIZE = "conn-wbuf-size";
        static constexpr const char* WRITE_BUFFER_SIZE_SPEC = "conn-wbuf-size";

        static constexpr const char* PUB_ACK_TIMEOUT = "pub-ack-timeout";
        static constexpr const char* PUB_ACK_TIMEOUT_SPEC = "pub-ack-timeout";

        static constexpr const char* PUB_MAX_RETRIES = "pub-max-retries";
        static constexpr const char* PUB_MAX_RETRIES_SPEC = "pub-max-retries";

        static constexpr const char* COMMAND_LINE_GROUP = "Command line";
        static constexpr const char* SERVER_GROUP = "Server";
        static constexpr const char* CONNECTION_GROUP = "Connection";
        static constexpr const char* LOGGING_GROUP = "Logging";
        static constexpr const char* PUBLICATION_GROUP = "Publication";
        static constexpr const char* AUTHENTICATION_GROUP = "Authentication";
        static const std::vector<std::string> GROUPS;

       public:
        [[nodiscard]] auto create( ) const -> cxxopts::Options;
    };  // class options_factory
}  // namespace io_wally::app
