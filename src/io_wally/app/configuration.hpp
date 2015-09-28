#pragma once

#include <iostream>
#include <fstream>
#include <cstdint>

#include <boost/program_options.hpp>

#include "io_wally/context.hpp"

using namespace std;
namespace options = boost::program_options;

namespace io_wally
{
    namespace app
    {
        namespace defaults
        {
            static const uint32_t DEFAULT_CONNECT_TIMEOUT_MS = 1000;

            static const size_t DEFAULT_INITIAL_READ_BUFFER_SIZE = 256;

            static const size_t DEFAULT_INITIAL_WRITE_BUFFER_SIZE = 256;

            static const string DEFAULT_CONFIG_FILE = "/etc/mqttd.conf";

            static const string DEFAULT_LOG_FILE = "/var/log/mqttd.log";

            static const string DEFAULT_SERVER_ADDRESS = "0.0.0.0";

            static const int DEFAULT_SERVER_PORT = 1883;

            static const string DEFAULT_AUTHENTICATION_SERVICE_FACTORY = "accept_all";
        }

        const pair<const options::variables_map, const options::options_description> parse_configuration( int argc,
                                                                                                          char** argv )
        {
            options::variables_map config;

            options::options_description cmd_line_opts( "Command line" );
            cmd_line_opts.add_options( )( io_wally::context::HELP, "Print help message and exit" )(
                io_wally::context::CONFIG_FILE,
                options::value<string>( )->default_value( defaults::DEFAULT_CONFIG_FILE ),
                "Use configuration from <file>" );

            options::options_description logging_opts( "Logging" );
            logging_opts.add_options( )( io_wally::context::LOG_FILE,
                                         options::value<string>( )->default_value( defaults::DEFAULT_LOG_FILE ),
                                         "Log to file <file>" )(
                io_wally::context::LOG_SYNC,
                options::bool_switch( ),
                "Whether logging should be synchronous (not recommended)" )(
                io_wally::context::LOG_CONSOLE, options::bool_switch( ), "Whether to log to console" );

            options::options_description server_opts( "Server" );
            server_opts.add_options( )( io_wally::context::SERVER_ADDRESS,
                                        options::value<string>( )->default_value( defaults::DEFAULT_SERVER_ADDRESS ),
                                        "Address server should listen on" )(
                io_wally::context::SERVER_PORT,
                options::value<int>( )->default_value( defaults::DEFAULT_SERVER_PORT ) );

            options::options_description authentication_opts( "Authentication" );
            authentication_opts.add_options( )(
                io_wally::context::AUTHENTICATION_SERVICE_FACTORY,
                options::value<string>( )->default_value( defaults::DEFAULT_AUTHENTICATION_SERVICE_FACTORY ),
                "Name of authentication service factory" );

            options::options_description connection_opts( "Connection" );
            connection_opts.add_options( )(
                io_wally::context::CONNECT_TIMEOUT,
                options::value<uint32_t>( )->default_value( defaults::DEFAULT_CONNECT_TIMEOUT_MS ),
                "Connect timeout in ms" )(
                io_wally::context::READ_BUFFER_SIZE,
                options::value<size_t>( )->default_value( defaults::DEFAULT_INITIAL_READ_BUFFER_SIZE ),
                "Initial read buffer size in bytes" )(
                io_wally::context::WRITE_BUFFER_SIZE,
                options::value<size_t>( )->default_value( defaults::DEFAULT_INITIAL_WRITE_BUFFER_SIZE ),
                "Initial write buffer size in bytes" );

            options::options_description all( "Allowed options" );
            all.add( cmd_line_opts ).add( logging_opts ).add( server_opts ).add( authentication_opts ).add(
                connection_opts );
            options::store( options::parse_command_line( argc, argv, all ), config );

            options::options_description config_file( "Config file options" );
            config_file.add( logging_opts ).add( server_opts ).add( authentication_opts ).add( connection_opts );
            ifstream config_fstream( config[io_wally::context::CONFIG_FILE].as<string>( ).c_str( ) );
            options::store( options::parse_config_file( config_fstream, config_file ), config );
            config_fstream.close( );  // TODO: Should be made exception-safe

            return make_pair( config, all );
        }
    }  // namespace app
}
