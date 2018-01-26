#include "io_wally/app/application.hpp"

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

#include <boost/program_options.hpp>

#include "io_wally/context.hpp"

#include "io_wally/logging/logging.hpp"

#include "io_wally/app/authentication_service_factories.hpp"

namespace io_wally
{
    namespace options = boost::program_options;

    namespace app
    {
        static const std::string USAGE = R"USG(
mqttd: MQTT 3.1.1 Broker v. 0.0.1-PREALPHA

mqttd is a fledgling MQTT 3.1.1 broker currently undergoing heavy development.
PLANNED features include:

 * Full support for the MQTT 3.1.1 specification (way to go)
 * DBus interface
 * SYS topics suport
 * Pluggable authentication providers

DISCLAIMER:

  THIS IS PRE-ALPHA SOFTWARE. It's woefully incomplete, doesn't hold water,
  and may, and WILL (on a bad day), crash your kernel, nuke your hard drive
  and insult your girl/boy friend. YOU HAVE BEEN WARNED.
)USG";

        int application::run( int argc, const char** argv )
        {
            try
            {
                auto config_plus_desc = options_parser_.parse( argc, argv );
                auto config = config_plus_desc.first;
                auto all_opts_desc = config_plus_desc.second;

                if ( config.count( context::HELP ) )
                {
                    std::cout << USAGE << std::endl << all_opts_desc << std::endl;
                    return EC_OK;
                }

                options::notify( config );

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
            catch ( const options::error& e )
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
