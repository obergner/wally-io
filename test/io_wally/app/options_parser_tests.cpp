#include "catch.hpp"

#include <string>
#include <utility>
#include <fstream>
#include <cstdint>

#include <boost/program_options.hpp>

#include "io_wally/defaults.hpp"
#include "io_wally/context.hpp"
#include "io_wally/app/options_parser.hpp"

SCENARIO( "options_parser", "[options]" )
{
    io_wally::app::options_parser under_test;

    GIVEN( "a command line with no options set" )
    {
        const char* command_line_args[]{"executable"};

        WHEN( "parsing that command line" )
        {
            const std::pair<const boost::program_options::variables_map,
                            const boost::program_options::options_description> config_plus_desc =
                under_test.parse( sizeof( command_line_args ) / sizeof( *command_line_args ), command_line_args );

            THEN( "it should return a variables_map with default options" )
            {
                const boost::program_options::variables_map config = config_plus_desc.first;

                CHECK( config[io_wally::context::CONFIG_FILE].as<const std::string>( ) ==
                       io_wally::defaults::DEFAULT_CONFIG_FILE );
                CHECK( config[io_wally::context::LOG_FILE].as<const std::string>( ) ==
                       io_wally::defaults::DEFAULT_LOG_FILE );
                CHECK( config[io_wally::context::LOG_CONSOLE].as<const bool>( ) == false );
                CHECK( config[io_wally::context::LOG_SYNC].as<const bool>( ) == false );
                CHECK( config[io_wally::context::SERVER_ADDRESS].as<const std::string>( ) ==
                       io_wally::defaults::DEFAULT_SERVER_ADDRESS );
                CHECK( config[io_wally::context::SERVER_PORT].as<const int>( ) ==
                       io_wally::defaults::DEFAULT_SERVER_PORT );
                CHECK( config[io_wally::context::AUTHENTICATION_SERVICE_FACTORY].as<const std::string>( ) ==
                       io_wally::defaults::DEFAULT_AUTHENTICATION_SERVICE_FACTORY );
                CHECK( config[io_wally::context::CONNECT_TIMEOUT].as<const std::uint32_t>( ) ==
                       io_wally::defaults::DEFAULT_CONNECT_TIMEOUT_MS );
                CHECK( config[io_wally::context::READ_BUFFER_SIZE].as<const std::size_t>( ) ==
                       io_wally::defaults::DEFAULT_INITIAL_READ_BUFFER_SIZE );
                REQUIRE( config[io_wally::context::WRITE_BUFFER_SIZE].as<const std::size_t>( ) ==
                         io_wally::defaults::DEFAULT_INITIAL_WRITE_BUFFER_SIZE );
            }
        }
    }

    GIVEN( "a command line with all supported options explicitly set" )
    {
        const char* const conf_file = "/etc/conf.file";
        const char* const log_file = "/var/log/log.file";
        const std::string server_address( "8.9.10.11" );
        const int server_port = 1234;
        const std::string auth_service_factory( "test_auth_srvc_factory" );
        const std::uint32_t connect_timeout_ms = 3456;
        const std::size_t read_buffer_size = 1024;
        const std::size_t write_buffer_size = 4096;

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
                                        "--auth-service-factory",
                                        "test_auth_srvc_factory",
                                        "--conn-timeout",
                                        "3456",
                                        "--conn-rbuf-size",
                                        "1024",
                                        "--conn-wbuf-size",
                                        "4096"};

        WHEN( "parsing that command line" )
        {
            const std::pair<const boost::program_options::variables_map,
                            const boost::program_options::options_description> config_plus_desc =
                under_test.parse( sizeof( command_line_args ) / sizeof( *command_line_args ), command_line_args );

            THEN( "it should return a variables_map with all command line args correctly parsed" )
            {
                const boost::program_options::variables_map config = config_plus_desc.first;

                CHECK( config[io_wally::context::CONFIG_FILE].as<const std::string>( ) == conf_file );
                CHECK( config[io_wally::context::LOG_FILE].as<const std::string>( ) == log_file );
                CHECK( config[io_wally::context::LOG_CONSOLE].as<const bool>( ) == true );
                CHECK( config[io_wally::context::LOG_SYNC].as<const bool>( ) == true );
                CHECK( config[io_wally::context::SERVER_ADDRESS].as<const std::string>( ) == server_address );
                CHECK( config[io_wally::context::SERVER_PORT].as<const int>( ) == server_port );
                CHECK( config[io_wally::context::AUTHENTICATION_SERVICE_FACTORY].as<const std::string>( ) ==
                       auth_service_factory );
                CHECK( config[io_wally::context::CONNECT_TIMEOUT].as<const std::uint32_t>( ) == connect_timeout_ms );
                CHECK( config[io_wally::context::READ_BUFFER_SIZE].as<const std::size_t>( ) == read_buffer_size );
                REQUIRE( config[io_wally::context::WRITE_BUFFER_SIZE].as<const std::size_t>( ) == write_buffer_size );
            }
        }
    }

    GIVEN( "a configuration file containing all supported options" )
    {
        const char* const conf_file = "./build/test/test.conf";

        const char* const log_file = "/var/tmp/config-file-log.log";
        const std::string server_address( "1.1.1.1" );
        const int server_port = 99;
        const std::string auth_service_factory( "config_file_auth_srvc_factory" );
        const std::uint32_t connect_timeout_ms = 12;
        const std::size_t read_buffer_size = 2345;
        const std::size_t write_buffer_size = 8999;

        const char* conf_file_contents = R"CONF(
log-file = /var/tmp/config-file-log.log
log-console = true
log-sync = true
server-address = 1.1.1.1
server-port = 99
auth-service-factory = config_file_auth_srvc_factory
conn-timeout = 12
conn-rbuf-size = 2345
conn-wbuf-size = 8999
)CONF";
        {
            std::ofstream conf_file_ofs( conf_file );

            conf_file_ofs << conf_file_contents;
        }

        const char* command_line_args[]{
            "executable", "--conf-file", conf_file,
        };

        WHEN( "parsing a command line pointing to that configuration file" )
        {
            const std::pair<const boost::program_options::variables_map,
                            const boost::program_options::options_description> config_plus_desc =
                under_test.parse( sizeof( command_line_args ) / sizeof( *command_line_args ), command_line_args );

            THEN( "it should return a variables_map with all options populated from configuration file" )
            {
                const boost::program_options::variables_map config = config_plus_desc.first;

                CHECK( config[io_wally::context::CONFIG_FILE].as<const std::string>( ) == conf_file );
                CHECK( config[io_wally::context::LOG_FILE].as<const std::string>( ) == log_file );
                CHECK( config[io_wally::context::LOG_CONSOLE].as<const bool>( ) == true );
                CHECK( config[io_wally::context::LOG_SYNC].as<const bool>( ) == true );
                CHECK( config[io_wally::context::SERVER_ADDRESS].as<const std::string>( ) == server_address );
                CHECK( config[io_wally::context::SERVER_PORT].as<const int>( ) == server_port );
                CHECK( config[io_wally::context::AUTHENTICATION_SERVICE_FACTORY].as<const std::string>( ) ==
                       auth_service_factory );
                CHECK( config[io_wally::context::CONNECT_TIMEOUT].as<const std::uint32_t>( ) == connect_timeout_ms );
                CHECK( config[io_wally::context::READ_BUFFER_SIZE].as<const std::size_t>( ) == read_buffer_size );
                REQUIRE( config[io_wally::context::WRITE_BUFFER_SIZE].as<const std::size_t>( ) == write_buffer_size );
            }
        }
    }
}
