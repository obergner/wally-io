#include "catch.hpp"

#include <cstdint>
#include <string>

#include <cxxopts.hpp>

#include "io_wally/app/options_factory.hpp"
#include "io_wally/context.hpp"
#include "io_wally/defaults.hpp"

SCENARIO( "options_factory", "[options]" )
{
    auto under_test = io_wally::app::options_factory{};

    GIVEN( "a command line with no options set" )
    {
        const char* command_line_args[]{"executable"};

        WHEN( "parsing that command line" )
        {
            auto argc = static_cast<int>( sizeof( command_line_args ) / sizeof( *command_line_args ) );
            auto argv = const_cast<char**>( command_line_args );
            auto opts = under_test.create( );
            auto config = opts.parse( argc, argv );

            THEN( "it should return a variables_map with default options" )
            {
                CHECK( config[io_wally::context::LOG_FILE].as<std::string>( ) == io_wally::defaults::DEFAULT_LOG_FILE );
                CHECK( config[io_wally::context::LOG_FILE_LEVEL].as<std::string>( ) ==
                       io_wally::defaults::DEFAULT_LOG_FILE_LEVEL );
                CHECK( config[io_wally::context::LOG_CONSOLE].as<bool>( ) == false );
                CHECK( config[io_wally::context::LOG_CONSOLE_LEVEL].as<std::string>( ) ==
                       io_wally::defaults::DEFAULT_LOG_CONSOLE_LEVEL );
                CHECK( config[io_wally::context::LOG_DISABLE].as<bool>( ) == false );
                CHECK( config[io_wally::context::SERVER_ADDRESS].as<std::string>( ) ==
                       io_wally::defaults::DEFAULT_SERVER_ADDRESS );
                CHECK( config[io_wally::context::SERVER_PORT].as<int>( ) == io_wally::defaults::DEFAULT_SERVER_PORT );
                CHECK( config[io_wally::context::AUTHENTICATION_SERVICE_FACTORY].as<std::string>( ) ==
                       io_wally::defaults::DEFAULT_AUTHENTICATION_SERVICE_FACTORY );
                CHECK( config[io_wally::context::CONNECT_TIMEOUT].as<std::uint32_t>( ) ==
                       io_wally::defaults::DEFAULT_CONNECT_TIMEOUT_MS );
                CHECK( config[io_wally::context::READ_BUFFER_SIZE].as<std::size_t>( ) ==
                       io_wally::defaults::DEFAULT_INITIAL_READ_BUFFER_SIZE );
                REQUIRE( config[io_wally::context::WRITE_BUFFER_SIZE].as<std::size_t>( ) ==
                         io_wally::defaults::DEFAULT_INITIAL_WRITE_BUFFER_SIZE );
            }
        }
    }

    GIVEN( "a command line with all supported options explicitly set" )
    {
        const char* const log_file = "/var/log/log.file";
        const char* const log_file_level = "fatal";
        const char* const log_console_level = "error";
        const std::string server_address( "8.9.10.11" );
        const int server_port = 1234;
        const std::string auth_service_factory( "test_auth_srvc_factory" );
        const std::uint32_t connect_timeout_ms = 3456;
        const std::size_t read_buffer_size = 1024;
        const std::size_t write_buffer_size = 4096;

        const char* command_line_args[]{"executable",
                                        "--log-file",
                                        log_file,
                                        "--log-file-level",
                                        log_file_level,
                                        "--log-console",
                                        "--log-console-level",
                                        log_console_level,
                                        "--log-disable",
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
            auto argc = static_cast<int>( sizeof( command_line_args ) / sizeof( *command_line_args ) );
            auto argv = const_cast<char**>( command_line_args );
            auto opts = under_test.create( );
            auto config = opts.parse( argc, argv );

            THEN( "it should return a variables_map with all command line args correctly parsed" )
            {
                CHECK( config[io_wally::context::LOG_FILE].as<std::string>( ) == log_file );
                CHECK( config[io_wally::context::LOG_FILE_LEVEL].as<std::string>( ) == log_file_level );
                CHECK( config[io_wally::context::LOG_CONSOLE].as<bool>( ) == true );
                CHECK( config[io_wally::context::LOG_CONSOLE_LEVEL].as<std::string>( ) == log_console_level );
                CHECK( config[io_wally::context::LOG_DISABLE].as<bool>( ) == true );
                CHECK( config[io_wally::context::SERVER_ADDRESS].as<std::string>( ) == server_address );
                CHECK( config[io_wally::context::SERVER_PORT].as<int>( ) == server_port );
                CHECK( config[io_wally::context::AUTHENTICATION_SERVICE_FACTORY].as<std::string>( ) ==
                       auth_service_factory );
                CHECK( config[io_wally::context::CONNECT_TIMEOUT].as<std::uint32_t>( ) == connect_timeout_ms );
                CHECK( config[io_wally::context::READ_BUFFER_SIZE].as<std::size_t>( ) == read_buffer_size );
                REQUIRE( config[io_wally::context::WRITE_BUFFER_SIZE].as<std::size_t>( ) == write_buffer_size );
            }
        }
    }
}
