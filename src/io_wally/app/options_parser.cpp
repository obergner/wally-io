#include <iostream>
#include <fstream>

#include "io_wally/defaults.hpp"
#include "io_wally/context.hpp"
#include "io_wally/app/options_parser.hpp"

using namespace io_wally::defaults;

namespace io_wally
{
    namespace app
    {
        const pair<const options::variables_map, const options::options_description> options_parser::parse(
            const int argc,
            const char** argv ) const
        {
            options::variables_map config;

            options::options_description cmd_line_opts( "Command line" );
            cmd_line_opts.add_options( )( io_wally::context::HELP, "Print help message and exit" )(
                io_wally::context::CONFIG_FILE,
                options::value<string>( )->default_value( DEFAULT_CONFIG_FILE ),
                "Use configuration from <arg>" );

            options::options_description logging_opts( "Logging" );
            logging_opts.add_options( )( io_wally::context::LOG_FILE,
                                         options::value<string>( )->default_value( DEFAULT_LOG_FILE ),
                                         "Log to file <arg>" )(
                io_wally::context::LOG_SYNC,
                options::bool_switch( ),
                "Whether logging should be synchronous (not recommended)" )(
                io_wally::context::LOG_CONSOLE, options::bool_switch( ), "Whether to log to console" );

            options::options_description server_opts( "Server" );
            server_opts.add_options( )( io_wally::context::SERVER_ADDRESS,
                                        options::value<string>( )->default_value( DEFAULT_SERVER_ADDRESS ),
                                        "Address server should listen on" )(
                io_wally::context::SERVER_PORT,
                options::value<int>( )->default_value( DEFAULT_SERVER_PORT ),
                "Port server should listen on" );

            options::options_description authentication_opts( "Authentication" );
            authentication_opts.add_options( )(
                io_wally::context::AUTHENTICATION_SERVICE_FACTORY,
                options::value<string>( )->default_value( DEFAULT_AUTHENTICATION_SERVICE_FACTORY ),
                "Name of authentication service factory" );

            options::options_description connection_opts( "Connection" );
            connection_opts.add_options( )( io_wally::context::CONNECT_TIMEOUT,
                                            options::value<uint32_t>( )->default_value( DEFAULT_CONNECT_TIMEOUT_MS ),
                                            "Connect timeout in ms" )(
                io_wally::context::READ_BUFFER_SIZE,
                options::value<size_t>( )->default_value( DEFAULT_INITIAL_READ_BUFFER_SIZE ),
                "Initial read buffer size in bytes" )(
                io_wally::context::WRITE_BUFFER_SIZE,
                options::value<size_t>( )->default_value( DEFAULT_INITIAL_WRITE_BUFFER_SIZE ),
                "Initial write buffer size in bytes" );

            options::options_description all( "Allowed options" );
            all.add( cmd_line_opts ).add( logging_opts ).add( server_opts ).add( authentication_opts ).add(
                connection_opts );
            options::store( options::parse_command_line( argc, argv, all ), config );

            options::options_description config_file( "Config file options" );
            config_file.add( logging_opts ).add( server_opts ).add( authentication_opts ).add( connection_opts );
            ifstream config_fstream( config[io_wally::context::CONFIG_FILE].as<string>( ).c_str( ) );
            options::store( options::parse_config_file( config_fstream, config_file ), config );

            return make_pair( config, all );
        }
    }  // namespace app
}
