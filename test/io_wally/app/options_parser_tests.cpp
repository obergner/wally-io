#include "catch.hpp"

#include "io_wally/defaults.hpp"
#include "io_wally/context.hpp"
#include "io_wally/app/options_parser.hpp"

using namespace io_wally;

SCENARIO( "options_parser", "[options]" )
{
    io_wally::app::options_parser under_test;

    GIVEN( "a command line with no options set" )
    {
        const char* command_line_args[]{"executable"};

        WHEN( "parsing that command line" )
        {
            const std::pair<const options::variables_map, const options::options_description> config_plus_desc =
                under_test.parse( sizeof( command_line_args ) / sizeof( *command_line_args ), command_line_args );

            THEN( "it should return a variables_map with default options" )
            {
                const options::variables_map config = config_plus_desc.first;

                CHECK( config[io_wally::context::CONFIG_FILE].as<const string>( ) ==
                       io_wally::defaults::DEFAULT_CONFIG_FILE );
                CHECK( config[io_wally::context::LOG_FILE].as<const string>( ) ==
                       io_wally::defaults::DEFAULT_LOG_FILE );
                CHECK( config[io_wally::context::LOG_CONSOLE].as<const bool>( ) == false );
                CHECK( config[io_wally::context::LOG_SYNC].as<const bool>( ) == false );
                CHECK( config[io_wally::context::SERVER_ADDRESS].as<const string>( ) ==
                       io_wally::defaults::DEFAULT_SERVER_ADDRESS );
                CHECK( config[io_wally::context::SERVER_PORT].as<const int>( ) ==
                       io_wally::defaults::DEFAULT_SERVER_PORT );
                CHECK( config[io_wally::context::AUTHENTICATION_SERVICE_FACTORY].as<const string>( ) ==
                       io_wally::defaults::DEFAULT_AUTHENTICATION_SERVICE_FACTORY );
                CHECK( config[io_wally::context::CONNECT_TIMEOUT].as<const uint32_t>( ) ==
                       io_wally::defaults::DEFAULT_CONNECT_TIMEOUT_MS );
                CHECK( config[io_wally::context::READ_BUFFER_SIZE].as<const size_t>( ) ==
                       io_wally::defaults::DEFAULT_INITIAL_READ_BUFFER_SIZE );
                REQUIRE( config[io_wally::context::WRITE_BUFFER_SIZE].as<const size_t>( ) ==
                         io_wally::defaults::DEFAULT_INITIAL_WRITE_BUFFER_SIZE );
            }
        }
    }

    GIVEN( "a command line with all supported options explicitly set" )
    {
        const char* const conf_file = "/etc/conf.file";
        const char* const log_file = "/var/log/log.file";
        const string server_address( "8.9.10.11" );
        const int server_port = 1234;
        const string auth_service_factory( "test_auth_srvc_factory" );
        const uint32_t connect_timeout_ms = 3456;
        const size_t read_buffer_size = 1024;
        const size_t write_buffer_size = 4096;

        const char* command_line_args[]{"executable",
                                        "--conf-file",
                                        conf_file,
                                        "--log-file",
                                        log_file,
                                        "--log-console",
                                        "--log-sync",
                                        "--server-address",
                                        "8.9.10.11",
                                        "--server-port",
                                        "1234",
                                        "--authentication-service-factory",
                                        "test_auth_srvc_factory",
                                        "--connect-timeout",
                                        "3456",
                                        "--read-buffer-size",
                                        "1024",
                                        "--write-buffer-size",
                                        "4096"};

        WHEN( "parsing that command line" )
        {
            const std::pair<const options::variables_map, const options::options_description> config_plus_desc =
                under_test.parse( sizeof( command_line_args ) / sizeof( *command_line_args ), command_line_args );

            THEN( "it should return a variables_map with all command line args correctly parsed" )
            {
                const options::variables_map config = config_plus_desc.first;

                CHECK( config[io_wally::context::CONFIG_FILE].as<const string>( ) == conf_file );
                CHECK( config[io_wally::context::LOG_FILE].as<const string>( ) == log_file );
                CHECK( config[io_wally::context::LOG_CONSOLE].as<const bool>( ) == true );
                CHECK( config[io_wally::context::LOG_SYNC].as<const bool>( ) == true );
                CHECK( config[io_wally::context::SERVER_ADDRESS].as<const string>( ) == server_address );
                CHECK( config[io_wally::context::SERVER_PORT].as<const int>( ) == server_port );
                CHECK( config[io_wally::context::AUTHENTICATION_SERVICE_FACTORY].as<const string>( ) ==
                       auth_service_factory );
                CHECK( config[io_wally::context::CONNECT_TIMEOUT].as<const uint32_t>( ) == connect_timeout_ms );
                CHECK( config[io_wally::context::READ_BUFFER_SIZE].as<const size_t>( ) == read_buffer_size );
                REQUIRE( config[io_wally::context::WRITE_BUFFER_SIZE].as<const size_t>( ) == write_buffer_size );
            }
        }
    }
}
