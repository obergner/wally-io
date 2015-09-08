#include "io_wally/logging.hpp"
#include "io_wally/mqtt_server.hpp"
#include "io_wally/impl/accept_all_authentication_service_factory.cpp"

static const std::string host = "0.0.0.0";
static const std::string port = "1883";

static const std::string log_file_prefix = ".testlog";

static io_wally::impl::accept_all_authentication_service_factory auth_service_factory;

int main( int /*  argc */, char** /* argv */ )
{
    io_wally::logging::init_logging( log_file_prefix, true );
    // auto auth_srvc = auth_service_factory( "ignored" );
    // const boost::optional<const string> usr = std::string( "anonymous" );
    // const boost::optional<const string> pwd = std::string( "secret" );
    // ( *auth_srvc )( "127.0.0.1", usr, pwd );

    io_wally::mqtt_server srv( host, port, auth_service_factory( "ignored" ) );
    srv.run( );

    return 0;
}
