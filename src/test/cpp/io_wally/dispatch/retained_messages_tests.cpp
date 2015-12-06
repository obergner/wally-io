#include "catch.hpp"

#include "framework/factories.hpp"

#include "io_wally/protocol/common.hpp"
#include "io_wally/dispatch/common.hpp"
#include "io_wally/dispatch/retained_messages.hpp"
#include "io_wally/protocol/publish_packet.hpp"

using namespace io_wally::protocol;

SCENARIO( "retained_messages#retain", "[dispatch]" )
{
    GIVEN( "a PUBLISH packet with retain flag set" )
    {
        io_wally::dispatch::retained_messages under_test{};

        auto const topic = "/test/retain";
        auto const publish = framework::create_publish_packet( topic, true );

        WHEN( "a caller stores this PUBLISH packet in retained_messages " )
        {
            under_test.retain( publish );

            THEN( "retained_messages#size() should return 1" )
            {
                REQUIRE( under_test.size( ) == 1 );
            }
        }
    }
}
