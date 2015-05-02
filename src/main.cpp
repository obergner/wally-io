#include "io_wally/mqtt_server.hpp"

static const std::string host = "127.0.0.1";
static const std::string port = "1883";

int main( int argc, char* argv[] )
{
    io_wally::mqtt_server srv( host, port );

    srv.run( );

    return 0;
}
