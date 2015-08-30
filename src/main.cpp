#include "io_wally/logging.hpp"
#include "io_wally/mqtt_server.hpp"
#include "io_wally/impl/default_packet_handler_factory.hpp"

static const std::string host = "0.0.0.0";
static const std::string port = "1883";

static const std::string log_file_prefix = ".testlog";

static const io_wally::impl::default_packet_handler_factory packet_handler_factory;

int main( int /*  argc */, char** /* argv */ )
{
    io_wally::logging::init_logging( log_file_prefix, true );

    io_wally::mqtt_server srv( host, port, packet_handler_factory );
    srv.run( );

    return 0;
}
