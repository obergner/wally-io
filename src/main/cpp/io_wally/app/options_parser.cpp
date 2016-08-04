#include "io_wally/app/options_parser.hpp"

#include <fstream>
#include <iostream>
#include <utility>

#include <boost/program_options.hpp>

#include "io_wally/defaults.hpp"

namespace io_wally
{
    using namespace std;
    namespace options = boost::program_options;

    namespace app
    {
        const pair<const options::variables_map, const options::options_description> options_parser::parse(
            const int argc,
            const char** argv ) const
        {
            using namespace io_wally::defaults;

            auto config = options::variables_map{};

            auto cmd_line_opts = options::options_description{"Command line", 100, 50};
            cmd_line_opts.add_options( )( HELP_SPEC, "Print help message and exit" )(
                CONFIG_FILE_SPEC,
                options::value<string>( )->default_value( DEFAULT_CONFIG_FILE )->value_name( "<file>" ),
                "Read configuration from <file>" );

            auto server_opts = options::options_description{"Server", 100, 50};
            server_opts.add_options( )(
                SERVER_ADDRESS_SPEC,
                options::value<string>( )->default_value( DEFAULT_SERVER_ADDRESS )->value_name( "<IP>" ),
                "Bind server to <IP>" )(
                SERVER_PORT_SPEC, options::value<int>( )->default_value( DEFAULT_SERVER_PORT )->value_name( "<port>" ),
                "Bind server to <port>" );

            auto connection_opts = options::options_description{"Connection", 100, 50};
            connection_opts.add_options( )(
                CONNECT_TIMEOUT_SPEC,
                options::value<uint32_t>( )->default_value( DEFAULT_CONNECT_TIMEOUT_MS )->value_name( "<timeout>" ),
                "Close new client connection if not receiving a CONNECT request within <timeout> ms" )(
                READ_BUFFER_SIZE_SPEC,
                options::value<size_t>( )->default_value( DEFAULT_INITIAL_READ_BUFFER_SIZE )->value_name( "<bytes>" ),
                "Use initial read buffer of size <bytes>" )(
                WRITE_BUFFER_SIZE_SPEC,
                options::value<size_t>( )->default_value( DEFAULT_INITIAL_WRITE_BUFFER_SIZE )->value_name( "<bytes>" ),
                "Use initial write buffer of size <bytes>" );

            auto logging_opts = options::options_description{"Logging", 100, 50};
            logging_opts.add_options( )(
                LOG_FILE_SPEC, options::value<string>( )->default_value( DEFAULT_LOG_FILE )->value_name( "<file>" ),
                "Direct log output to <file>" )(
                LOG_FILE_LEVEL_SPEC,
                options::value<string>( )->default_value( DEFAULT_LOG_FILE_LEVEL )->value_name( "<level>" ),
                "Restrict file log output to <level> or above:\n  (trace|debug|info|warning|error|fatal)" )(
                LOG_SYNC_SPEC, options::bool_switch( ), "Use synchronous logging (not recommended)" )(
                LOG_CONSOLE_SPEC, options::bool_switch( ), "Log to console" )(
                LOG_CONSOLE_LEVEL_SPEC,
                options::value<string>( )->default_value( DEFAULT_LOG_CONSOLE_LEVEL )->value_name( "<level>" ),
                "Restrict console log output to <level> or above:\n  (trace|debug|info|warning|error|fatal)" )(
                LOG_DISABLE_SPEC, options::bool_switch( ),
                "Do not log, neither to file nor to console\nIf this option is set --log-file, --log-console, "
                "--log-file-level and --log-console-level will be ignored" );

            auto publication_opts = options::options_description{"Publication", 100, 50};
            publication_opts.add_options( )(
                PUB_ACK_TIMEOUT_SPEC,
                options::value<uint32_t>( )->default_value( DEFAULT_PUB_ACK_TIMEOUT_MS )->value_name( "<timeout>" ),
                "Resend PUBLISH after <timeout> ms without receiving a PUBACK" )(
                PUB_MAX_RETRIES_SPEC,
                options::value<size_t>( )->default_value( DEFAULT_PUB_MAX_RETRIES )->value_name( "<retries>" ),
                "Retry sending PUBLISH for at most <retries> times" );

            auto authentication_opts = options::options_description{"Authentication", 100, 50};
            authentication_opts.add_options( )( AUTHENTICATION_SERVICE_FACTORY_SPEC,
                                                options::value<string>( )
                                                    ->default_value( DEFAULT_AUTHENTICATION_SERVICE_FACTORY )
                                                    ->value_name( "<name>" ),
                                                "Use authentication service factory <name>" );

            auto all = options::options_description{"Allowed options", 100, 50};
            all.add( cmd_line_opts )
                .add( logging_opts )
                .add( server_opts )
                .add( publication_opts )
                .add( authentication_opts )
                .add( connection_opts );
            options::store( options::parse_command_line( argc, argv, all ), config );

            auto config_file = options::options_description{"Config file options", 100, 50};
            config_file.add( logging_opts )
                .add( server_opts )
                .add( publication_opts )
                .add( authentication_opts )
                .add( connection_opts );
            auto config_fstream = ifstream{config[CONFIG_FILE].as<string>( ).c_str( )};
            options::store( options::parse_config_file( config_fstream, config_file ), config );

            return make_pair( config, all );
        }
    }  // namespace app
}
