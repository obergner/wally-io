#include "io_wally/app/application.hpp"
#include "io_wally/app/logging.hpp"
#include "io_wally/mqtt_server.hpp"
#include "io_wally/impl/accept_all_authentication_service_factory.hpp"

static const std::string host = "0.0.0.0";
static const std::string port = "1883";

static const std::string log_file_prefix = ".testlog";

static io_wally::impl::accept_all_authentication_service_factory auth_service_factory;
static options::variables_map config;

static io_wally::app::application& application = io_wally::app::application::instance( );

int main( int argc, char** argv )
{
    // io_wally::app::init_logging( config );

    // io_wally::mqtt_server srv( host, port, auth_service_factory( config ) );
    // srv.run( );

    // return 0;
    return application.run( argc, argv );
}
