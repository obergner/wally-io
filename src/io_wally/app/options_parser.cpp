#include "io_wally/app/options_parser.hpp"

#include <utility>
#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include "io_wally/defaults.hpp"

namespace io_wally
{
    namespace options = boost::program_options;

    namespace app
    {
        const pair<const options::variables_map, const options::options_description> options_parser::parse(
            const int argc,
            const char** argv ) const
        {
            using namespace io_wally::defaults;

            options::variables_map config;

            options::options_description cmd_line_opts( "Command line", 100, 50 );
            cmd_line_opts.add_options( )( HELP_SPEC, "Print help message and exit" )(
                CONFIG_FILE_SPEC,
                options::value<string>( )->default_value( DEFAULT_CONFIG_FILE )->value_name( "<file>" ),
                "Read configuration from <file>" );

            options::options_description server_opts( "Server", 100, 50 );
            server_opts.add_options( )(
                SERVER_ADDRESS_SPEC,
                options::value<string>( )->default_value( DEFAULT_SERVER_ADDRESS )->value_name( "<IP>" ),
                "Bind server to <IP>" )(
                SERVER_PORT_SPEC,
                options::value<int>( )->default_value( DEFAULT_SERVER_PORT )->value_name( "<port>" ),
                "Bind server to <port>" );

            options::options_description connection_opts( "Connection", 100, 50 );
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

            options::options_description logging_opts( "Logging", 100, 50 );
            logging_opts.add_options( )(
                LOG_FILE_SPEC,
                options::value<string>( )->default_value( DEFAULT_LOG_FILE )->value_name( "<file>" ),
                "Direct log output to <file>" )(
                LOG_FILE_LEVEL_SPEC,
                options::value<string>( )->default_value( DEFAULT_LOG_FILE_LEVEL )->value_name( "<level>" ),
                "Restrict file log output to <level> or above:\n  (trace|debug|info|warning|error|fatal)" )(
                LOG_SYNC_SPEC, options::bool_switch( ), "Use synchronous logging (not recommended)" )(
                LOG_CONSOLE_SPEC, options::bool_switch( ), "Log to console" )(
                LOG_CONSOLE_LEVEL_SPEC,
                options::value<string>( )->default_value( DEFAULT_LOG_CONSOLE_LEVEL )->value_name( "<level>" ),
                "Restrict console log output to <level> or above:\n  (trace|debug|info|warning|error|fatal)" )(
                LOG_DISABLE_SPEC,
                options::bool_switch( ),
                "Do not log, neither to file nor to console\nIf this option is set --log-file, --log-console, "
                "--log-file-level and --log-console-level will be ignored" );

            options::options_description authentication_opts( "Authentication", 100, 50 );
            authentication_opts.add_options( )(
                AUTHENTICATION_SERVICE_FACTORY_SPEC,
                options::value<string>( )->default_value( DEFAULT_AUTHENTICATION_SERVICE_FACTORY )->value_name(
                    "<name>" ),
                "Use authentication service factory <name>" );

            options::options_description all( "Allowed options", 100, 50 );
            all.add( cmd_line_opts ).add( logging_opts ).add( server_opts ).add( authentication_opts ).add(
                connection_opts );
            options::store( options::parse_command_line( argc, argv, all ), config );

            options::options_description config_file( "Config file options", 100, 50 );
            config_file.add( logging_opts ).add( server_opts ).add( authentication_opts ).add( connection_opts );
            ifstream config_fstream( config[CONFIG_FILE].as<string>( ).c_str( ) );
            options::store( options::parse_config_file( config_fstream, config_file ), config );

            return make_pair( config, all );
        }
    }  // namespace app
}
