#include "io_wally/app/options_parser.hpp"

#include <utility>
#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include "io_wally/defaults.hpp"
#include "io_wally/context.hpp"

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

            options::options_description cmd_line_opts( "Command line" );
            cmd_line_opts.add_options( )( context::HELP, "Print help message and exit" )(
                context::CONFIG_FILE,
                options::value<string>( )->default_value( DEFAULT_CONFIG_FILE )->value_name( "<file>" ),
                "Read configuration from <file>" );

            options::options_description logging_opts( "Logging" );
            logging_opts.add_options( )(
                context::LOG_FILE,
                options::value<string>( )->default_value( DEFAULT_LOG_FILE )->value_name( "<file>" ),
                "Direct log output to <file>" )(
                context::LOG_SYNC, options::bool_switch( ), "Use synchronous logging (not recommended)" )(
                context::LOG_CONSOLE, options::bool_switch( ), "Log to console" );

            options::options_description server_opts( "Server" );
            server_opts.add_options( )(
                context::SERVER_ADDRESS,
                options::value<string>( )->default_value( DEFAULT_SERVER_ADDRESS )->value_name( "<IP>" ),
                "Bind server to <IP>" )(
                context::SERVER_PORT,
                options::value<int>( )->default_value( DEFAULT_SERVER_PORT )->value_name( "<port>" ),
                "Bind server to <port>" );

            options::options_description authentication_opts( "Authentication" );
            authentication_opts.add_options( )(
                context::AUTHENTICATION_SERVICE_FACTORY,
                options::value<string>( )->default_value( DEFAULT_AUTHENTICATION_SERVICE_FACTORY )->value_name(
                    "<name>" ),
                "Use authentication service factory <name>" );

            options::options_description connection_opts( "Connection" );
            connection_opts.add_options( )(
                context::CONNECT_TIMEOUT,
                options::value<uint32_t>( )->default_value( DEFAULT_CONNECT_TIMEOUT_MS )->value_name( "<timeout>" ),
                "Close new client connection if not receiving a CONNECT request within <timeout> ms" )(
                context::READ_BUFFER_SIZE,
                options::value<size_t>( )->default_value( DEFAULT_INITIAL_READ_BUFFER_SIZE )->value_name( "<bytes>" ),
                "Use initial read buffer of size <bytes>" )(
                context::WRITE_BUFFER_SIZE,
                options::value<size_t>( )->default_value( DEFAULT_INITIAL_WRITE_BUFFER_SIZE ),
                "Use initial write buffer of size <bytes>" );

            options::options_description all( "Allowed options" );
            all.add( cmd_line_opts ).add( logging_opts ).add( server_opts ).add( authentication_opts ).add(
                connection_opts );
            options::store( options::parse_command_line( argc, argv, all ), config );

            //            for ( auto it = config.begin( ); it != config.end( ); ++it )
            //            {
            //                std::string raw_option_name = it->first;
            //                options::variable_value option_value = it->second;
            //
            //                raw_option_name.replace( 0, 1, );
            //            }

            options::options_description config_file( "Config file options" );
            config_file.add( logging_opts ).add( server_opts ).add( authentication_opts ).add( connection_opts );
            ifstream config_fstream( config[context::CONFIG_FILE].as<string>( ).c_str( ) );
            options::store( options::parse_config_file( config_fstream, config_file ), config );

            return make_pair( config, all );
        }
    }  // namespace app
}
