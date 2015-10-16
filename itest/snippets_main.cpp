#include "io_wally/error/protocol.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/subscription.hpp"

using namespace io_wally::protocol;

int main( int /* argc */, char** /* argv */ )
{
    const std::string topic_filter( "" );
    try
    {
        subscription( topic_filter, packet::QoS::AT_MOST_ONCE );
    }
    catch ( const std::exception& e )
    {
        std::cerr << "CAUGHT: ***** " << e.what( ) << std::endl;
        std::cerr.flush( );
    }
}
