#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include <stdexcept>
#include <iostream>
#include <thread>
#include <functional>

#include "io_wally/app/application.hpp"

int main( int argc, char* const argv[] )
{
    int result = 0;

    pid_t proc_pid = fork( );
    if ( proc_pid > 0 )
    {
        // Parent process
        sleep( 20 );

        try
        {
            result = Catch::Session( ).run( argc, argv );
        }
        catch ( const std::exception& e )
        {
            result = -1;
            std::cerr << "Running integration tests failed: " << e.what( ) << std::endl << std::flush;
        }

        std::cout << "Sending WallyIO MQTT server SIGTERM (integration tests) ..." << std::endl << std::flush;
        kill( proc_pid, SIGTERM );
        std::cout << "SIGTERM sent to WallyIO MQTT server (integration tests). Waiting for child process to exit ..."
                  << std::endl << std::flush;

        int status;
        while ( waitpid( proc_pid, &status, 0 ) == -1 )
            ;
        std::cout << " WallyIO MQTT server (integration tests) STOPPED." << std::endl << std::flush;
    }
    else if ( proc_pid == 0 )
    {
        try
        {
            // Child process
            const char* const log_file = "./target/itest/itest_log";
            const char* const log_file_level = "trace";
            const char* command_line_args[]{"executable", "--log-file", log_file, "--log-file-level", log_file_level};

            std::cout << "Starting WallyIO MQTT server (integration tests)" << std::endl << std::flush;
            io_wally::app::application app{};
            app.run( sizeof( command_line_args ) / sizeof( *command_line_args ), command_line_args );

            result = 0;
        }
        catch ( const std::exception& e )
        {
            result = -1;
            std::cerr << "Running MQTT server failed: " << e.what( ) << std::endl << std::flush;
        }
    }
    else
    {
        // Fork failed
        std::cerr << "Fork failed" << std::endl << std::flush;

        result = -1;
    }

    return result;
}
