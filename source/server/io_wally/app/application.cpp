#include "io_wally/app/application.hpp"

#include <iostream>
#include <mutex>
#include <string>

#include <cxxopts.hpp>

#include "io_wally/app/authentication_service_factories.hpp"
#include "io_wally/app/options_factory.hpp"
#include "io_wally/context.hpp"
#include "io_wally/logging/logging.hpp"

namespace io_wally
{
    namespace app
    {
        int application::run( int argc, char** argv )
        {
            try
            {
                auto cli = options_factory_.create( );
                const auto config = cli.parse( argc, argv );
                if ( config.count( context::HELP ) )
                {
                    std::cout << cli.help( options_factory::GROUPS ) << std::endl;
                    return EC_OK;
                }

                auto logger_factory = logging::logger_factory::create( config );

                auto auth_service_factory =
                    app::authentication_service_factories::instance( )[config[context::AUTHENTICATION_SERVICE_FACTORY]
                                                                           .as<std::string>( )];
                auto auth_service = auth_service_factory( config );

                auto ctx = context( std::move( config ), std::move( auth_service ), std::move( logger_factory ) );

                server_ = mqtt_server::create( std::move( ctx ) );
                {
                    // Nested scope to reliable release lock before we call server_.run(), which will block "forever".
                    auto ul = std::unique_lock<std::mutex>{startup_mutex_};

                    startup_completed_.notify_all( );
                }
                server_->run( );

                server_->wait_until_connections_closed( );

                server_->stop( "Server has completed shutdown sequence" );

                server_->wait_until_stopped( );
            }
            catch ( const cxxopts::OptionException& e )
            {
                std::cerr << "Wrong usage: " << e.what( ) << std::endl;
                return EC_MALFORMED_CMDLINE;
            }
            catch ( const std::exception& e )
            {
                std::cerr << "Error: " << e.what( ) << std::endl;
                return EC_RUNTIME_ERROR;
            }
            return EC_OK;
        }

        void application::wait_until_started( )
        {
            {
                // Nested block: we don't want to hold this lock when calling wait_for_bound() below
                auto ul = std::unique_lock<std::mutex>{startup_mutex_};

                startup_completed_.wait( ul, [this]( ) { return server_.use_count( ) > 0; } );
            }

            server_->wait_until_bound( );
        }

        void application::stop( const std::string& message )
        {
            server_->close_connections( message );
            server_->wait_until_connections_closed( );
            server_->stop( message );
            server_->wait_until_stopped( );
        }
    }  // namespace app
}
