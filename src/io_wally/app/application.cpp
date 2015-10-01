#include <iostream>
#include <fstream>

#include <io_wally/context.hpp>

#include <io_wally/app/application.hpp>
#include <io_wally/app/logging.hpp>
#include <io_wally/app/authentication_service_factories.hpp>

namespace io_wally
{
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

                if ( config.count( io_wally::context::HELP ) )
                {
                    cout << "MQTT 3.1.1 server" << endl << all_opts_desc << endl;
                    return application::EC_OK;
                }

                options::notify( config );

                io_wally::app::init_logging( config );

                const io_wally::spi::authentication_service_factory& auth_service_factory =
                    io_wally::app::authentication_service_factories::instance( )
                        [config[io_wally::context::AUTHENTICATION_SERVICE_FACTORY].as<string>( )];
                unique_ptr<io_wally::spi::authentication_service> auth_service = auth_service_factory( config );

                io_wally::context context( move( config ), move( auth_service ) );
                server_.reset( new io_wally::mqtt_server( move( context ) ) );

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
