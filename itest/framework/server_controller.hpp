#pragma once

#include <csignal>
#include <unistd.h>

#include "io_wally/app/application.hpp"

namespace framework
{
    namespace
    {
        void run_application( const char** command_line_args )
        {
            io_wally::app::application::instance( ).run( sizeof( command_line_args ) / sizeof( *command_line_args ),
                                                         command_line_args );
        }
    }  // namespace

    class server_controller
    {
       public:
        server_controller( )
        {
            const char* const log_file = "./build/itest/itest_log";
            const char* const log_file_level = "trace";
            const char* command_line_args[]{"executable", "--log-file", log_file, "--log-file-level", log_file_level};

            child_pid = fork( );
            if ( child_pid == 0 )
            {
                run_application( command_line_args );
            }
        }

        virtual ~server_controller( )
        {
            kill( child_pid, SIGKILL );
        }

       private:
        pid_t child_pid;
    };  // class server_controller

}  // namespace framework
