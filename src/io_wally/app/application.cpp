#include "io_wally/app/application.hpp"

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include "io_wally/context.hpp"

#include "io_wally/app/logging.hpp"
#include "io_wally/app/authentication_service_factories.hpp"

namespace io_wally
{
    namespace options = boost::program_options;

    namespace app
    {

        int application::run( int argc, char** argv )
        {
            try
            {
                const pair<const options::variables_map, const options::options_description> config_plus_desc =
                    options_parser_.parse( argc, const_cast<const char**>( argv ) );
                options::variables_map config = config_plus_desc.first;
                const options::options_description all_opts_desc = config_plus_desc.second;

                if ( config.count( context::HELP ) )
                {
                    cout << "MQTT 3.1.1 server" << endl << all_opts_desc << endl;
                    return application::EC_OK;
                }

                options::notify( config );

                app::init_logging( config );

                const spi::authentication_service_factory& auth_service_factory =
                    app::authentication_service_factories::instance( )[config[context::AUTHENTICATION_SERVICE_FACTORY]
                                                                           .as<string>( )];
                unique_ptr<spi::authentication_service> auth_service = auth_service_factory( config );

                context context( move( config ), move( auth_service ) );
                server_.reset( new mqtt_server( move( context ) ) );

                server_->run( );
            }
            catch ( const options::error& e )
            {
                cerr << "Wrong usage: " << e.what( ) << endl;
                return application::EC_MALFORMED_CMDLINE;
            }
            catch ( const std::exception& e )
            {
                cerr << "Error: " << e.what( ) << endl;
                return application::EC_RUNTIME_ERROR;
            }
            return application::EC_OK;
        }
    }  // namespace app
}
