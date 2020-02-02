#include "io_wally/app/options_factory.hpp"

#include <string>
#include <vector>

#include <cxxopts.hpp>

#include "io_wally/defaults.hpp"

namespace io_wally::app
{
    static const std::string USAGE = R"USG(

mqttd: MQTT 3.1.1 Broker v. 0.0.1-PREALPHA

mqttd is a fledgling MQTT 3.1.1 broker currently undergoing heavy development.
PLANNED features include:

 * Full support for the MQTT 3.1.1 specification (way to go)
 * GRPc management interface
 * Comprehensive stats
 * Pluggable authentication providers

DISCLAIMER:

  THIS IS PRE-ALPHA SOFTWARE. It's woefully incomplete, doesn't hold water,
  and may, and WILL (on a bad day), crash your kernel, nuke your hard drive
  and insult your girl/boy friend. YOU HAVE BEEN WARNED.
)USG";

    const std::vector<std::string> options_factory::GROUPS = {
        options_factory::COMMAND_LINE_GROUP, options_factory::SERVER_GROUP,      options_factory::CONNECTION_GROUP,
        options_factory::LOGGING_GROUP,      options_factory::PUBLICATION_GROUP, options_factory::AUTHENTICATION_GROUP};

    auto options_factory::create( ) const -> cxxopts::Options
    {
        using namespace io_wally::defaults;

        auto options = cxxopts::Options{"wally-iod", "A lightweight MQTT 3.1.1 server in the making\n"};
        options.custom_help( USAGE );

        // clang-format off
            options.add_options( COMMAND_LINE_GROUP )
                ( HELP_SPEC, "Print help message and exit" );

            options.add_options( SERVER_GROUP )
                ( SERVER_ADDRESS_SPEC, 
                  "Bind server to <IP>",
                  cxxopts::value<std::string>( )->default_value( DEFAULT_SERVER_ADDRESS ),
                  "<IP>" )
                ( SERVER_PORT_SPEC, 
                  "Bind server to <port>",
                  cxxopts::value<int>( )->default_value( std::to_string( DEFAULT_SERVER_PORT ) ), 
                  "<port>" );

            options.add_options( CONNECTION_GROUP )
                ( CONNECT_TIMEOUT_SPEC,
                  "Close new client connection if not receiving a CONNECT request within <timeout> ms",
                  cxxopts::value<uint32_t>( )->default_value( std::to_string( DEFAULT_CONNECT_TIMEOUT_MS ) ),
                  "<timeout>" )
                ( READ_BUFFER_SIZE_SPEC, 
                  "Use initial read buffer of size <bytes>",
                  cxxopts::value<size_t>( )->default_value( std::to_string( DEFAULT_INITIAL_READ_BUFFER_SIZE ) ),
                  "<bytes>" )
                ( WRITE_BUFFER_SIZE_SPEC, 
                  "Use initial write buffer of size <bytes>",
                  cxxopts::value<size_t>( )->default_value( std::to_string( DEFAULT_INITIAL_WRITE_BUFFER_SIZE ) ),
                  "<bytes>" );

            options.add_options( LOGGING_GROUP ) 
                ( LOG_FILE_SPEC, 
                  "Direct log output to <file>",
                  cxxopts::value<std::string>( )->default_value( DEFAULT_LOG_FILE ),
                  "<file>" )
                ( LOG_CONSOLE_SPEC, 
                  "Log to console", 
                  cxxopts::value<bool>( )->default_value( "false" )->implicit_value( "true" ) )
                ( LOG_LEVEL_SPEC,
                  "Restrict log output to <level> or above: (trace|debug|info|warn|err|critical)",
                  cxxopts::value<std::string>( )->default_value( DEFAULT_LOG_LEVEL ), "<level>" )
                ( LOG_DISABLE_SPEC,
                  "Do not log, neither to file nor to console. If this option is set --log-file, --log-console, "
                  "and --log-level will be ignored",
                  cxxopts::value<bool>( )->default_value( "false" )->implicit_value( "true" ) );

            options.add_options( PUBLICATION_GROUP )
                ( PUB_ACK_TIMEOUT_SPEC, 
                  "Resend PUBLISH after <timeout> ms without receiving a PUBACK",
                  cxxopts::value<uint32_t>( )->default_value( std::to_string( DEFAULT_PUB_ACK_TIMEOUT_MS ) ),
                  "<timeout>" )
                ( PUB_MAX_RETRIES_SPEC, 
                  "Retry sending PUBLISH for at most <retries> times",
                  cxxopts::value<size_t>( )->default_value( std::to_string( DEFAULT_PUB_MAX_RETRIES ) ),
                  "<retries>" );

            options.add_options( AUTHENTICATION_GROUP )
                ( AUTHENTICATION_SERVICE_FACTORY_SPEC,
                  "Use authentication service factory <name>",
                  cxxopts::value<std::string>( )->default_value( DEFAULT_AUTHENTICATION_SERVICE_FACTORY ),
                  "<name>" );
        // clang-format on

        return options;
    }
}  // namespace io_wally::app
