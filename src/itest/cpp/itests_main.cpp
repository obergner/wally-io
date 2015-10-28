#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include <string>
#include <stdexcept>
#include <iostream>

#include "io_wally/app/application.hpp"

namespace
{
    static const std::string prefix = "[itest] ";

    void log_info( const std::string msg )
    {
        std::cout << prefix << "--------------------------------------------------------------------------------"
                  << std::endl << std::flush;
        std::cout << prefix << msg << std::endl << std::flush;
        std::cout << prefix << "--------------------------------------------------------------------------------"
                  << std::endl << std::flush;
    }

    void log_error( const std::string msg )
    {
        std::cerr << prefix << "--------------------------------------------------------------------------------"
                  << std::endl << std::flush;
        std::cerr << prefix << msg << std::endl << std::flush;
        std::cerr << prefix << "--------------------------------------------------------------------------------"
                  << std::endl << std::flush;
    }

    int run_system_under_test( )
    {
        auto result = int{};
        try
        {
            // Child process
            const char* const log_file = "./target/itest/itest_log";
            const char* const log_file_level = "trace";
            const char* command_line_args[]{"executable", "--log-file", log_file, "--log-file-level", log_file_level};

            log_info( "Starting WallyIO MQTT server" );

            io_wally::app::application app{};
            app.run( sizeof( command_line_args ) / sizeof( *command_line_args ), command_line_args );

            result = 0;
        }
        catch ( const std::exception& e )
        {
            result = -1;
            log_error( "Running MQTT server failed: " + std::string( e.what( ) ) );
        }

        return result;
    }

    int run_catch( int argc, char* const argv[] )
    {
        auto result = int{};
        try
        {
            result = Catch::Session( ).run( argc, argv );
        }
        catch ( const std::exception& e )
        {
            result = -1;
            log_error( "Running integration tests threw exception: " + std::string( e.what( ) ) );
        }

        return result;
    }

    void terminate_system_under_test( const pid_t proc_pid )
    {
        log_info( "Sending WallyIO MQTT server SIGTERM ..." );
        kill( proc_pid, SIGTERM );
        log_info( "SIGTERM sent to WallyIO MQTT server. Waiting for child process to exit ..." );

        int status;
        while ( waitpid( proc_pid, &status, 0 ) == -1 )
            ;
        log_info( "WallyIO MQTT server STOPPED." );
    }
}

int main( int argc, char* const argv[] )
{
    int result = 0;

    pid_t proc_pid = fork( );
    if ( proc_pid > 0 )
    {
        // Parent process
        sleep( 2 );

        run_catch( argc, argv );

        terminate_system_under_test( proc_pid );
    }
    else if ( proc_pid == 0 )
    {
        run_system_under_test( );
    }
    else
    {
        // Fork failed
        std::cerr << "Fork failed" << std::endl << std::flush;

        result = -1;
    }

    return result;
}
