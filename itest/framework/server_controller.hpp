#pragma once

#include <iostream>
#include <thread>
#include <functional>

#include "io_wally/app/application.hpp"

namespace framework
{
    class server_controller
    {
       public:
        server_controller( )
        {
            const char* const log_file = "./build/itest/itest_log";
            const char* const log_file_level = "trace";
            const char* command_line_args[]{"executable", "--log-file", log_file, "--log-file-level", log_file_level};

            app_thread_ = std::thread( std::bind( &server_controller::run, this, command_line_args ) );
        }

        virtual ~server_controller( )
        {
            shutdown( );
        }

       private:
        void run( const char** command_line_args )
        {
            io_wally::app::application::instance( ).run( sizeof( command_line_args ) / sizeof( *command_line_args ),
                                                         command_line_args );
        }

        void shutdown( )
        {
            io_wally::app::application::instance( ).shutdown( "Integration test run done" );

            app_thread_.join( );
        }

       private:
        std::thread app_thread_;
    };  // class server_controller

}  // namespace framework
