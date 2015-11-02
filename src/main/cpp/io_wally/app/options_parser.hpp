#pragma once

#include <utility>

#include <boost/program_options.hpp>

namespace io_wally
{
    namespace app
    {
        /// \brief Utility class for reading program options from command line and configuration file.
        class options_parser final
        {
           public:
            static constexpr const char* HELP = "help";
            static constexpr const char* HELP_SPEC = "help,h";

            static constexpr const char* CONFIG_FILE = "conf-file";
            static constexpr const char* CONFIG_FILE_SPEC = "conf-file,c";

            static constexpr const char* LOG_FILE = "log-file";
            static constexpr const char* LOG_FILE_SPEC = "log-file,f";

            static constexpr const char* LOG_FILE_LEVEL = "log-file-level";
            static constexpr const char* LOG_FILE_LEVEL_SPEC = "log-file-level";

            static constexpr const char* LOG_CONSOLE = "log-console";
            static constexpr const char* LOG_CONSOLE_SPEC = "log-console";

            static constexpr const char* LOG_CONSOLE_LEVEL = "log-console-level";
            static constexpr const char* LOG_CONSOLE_LEVEL_SPEC = "log-console-level";

            static constexpr const char* LOG_SYNC = "log-sync";
            static constexpr const char* LOG_SYNC_SPEC = "log-sync";

            static constexpr const char* LOG_DISABLE = "log-disable";
            static constexpr const char* LOG_DISABLE_SPEC = "log-disable,s";

            static constexpr const char* SERVER_ADDRESS = "server-address";
            static constexpr const char* SERVER_ADDRESS_SPEC = "server-address,l";

            static constexpr const char* SERVER_PORT = "server-port";
            static constexpr const char* SERVER_PORT_SPEC = "server-port,p";

            static constexpr const char* AUTHENTICATION_SERVICE_FACTORY = "auth-service-factory";
            static constexpr const char* AUTHENTICATION_SERVICE_FACTORY_SPEC = "auth-service-factory";

            static constexpr const char* CONNECT_TIMEOUT = "conn-timeout";
            static constexpr const char* CONNECT_TIMEOUT_SPEC = "conn-timeout,t";

            static constexpr const char* READ_BUFFER_SIZE = "conn-rbuf-size";
            static constexpr const char* READ_BUFFER_SIZE_SPEC = "conn-rbuf-size";

            static constexpr const char* WRITE_BUFFER_SIZE = "conn-wbuf-size";
            static constexpr const char* WRITE_BUFFER_SIZE_SPEC = "conn-wbuf-size";

            static constexpr const char* PUB_ACK_TIMEOUT = "pub-ack-timeout";
            static constexpr const char* PUB_ACK_TIMEOUT_SPEC = "pub-ack-timeout";

            static constexpr const char* PUB_MAX_RETRIES = "pub-max-retries";
            static constexpr const char* PUB_MAX_RETRIES_SPEC = "pub-max-retries";

           public:
            const std::pair<const boost::program_options::variables_map,
                            const boost::program_options::options_description>
            parse( const int argc, const char** argv ) const;

        };  // class options_parser
    }       // namespace app
}
