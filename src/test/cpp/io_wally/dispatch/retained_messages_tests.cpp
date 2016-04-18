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

        WHEN( "no messages have been retained yet" )
        {
            THEN( "retained_messages#size() should return 0" )
            {
                REQUIRE( under_test.size( ) == 0 );
            }
        }

        WHEN( "a caller stores this PUBLISH packet in retained_messages " )
        {
            under_test.retain( publish );

            THEN( "retained_messages#size() should return 1" )
            {
                REQUIRE( under_test.size( ) == 1 );
            }
        }

        WHEN( "a caller stores this PUBLISH packet in retained_messages twice" )
        {
            under_test.retain( publish );
            under_test.retain( publish );

            THEN( "retained_messages#size() should still return 1" )
            {
                REQUIRE( under_test.size( ) == 1 );
            }
        }
    }

    GIVEN( "a PUBLISH packet with retain flag set and zero-length payload" )
    {
        io_wally::dispatch::retained_messages under_test{};

        auto const topic = "/test/retain";
        auto const publish_non_zero =
            framework::create_publish_packet( topic, true, std::vector<uint8_t>{'n', 'o', 'n', 'z', 'e', 'r', 'o'} );
        auto const publish_zero = framework::create_publish_packet( topic, true, std::vector<uint8_t>{} );

        WHEN(
            "a caller first stores a non-zero-length PUBLISH packet and then a zero-length PUBLISH packet for the same "
            "topic" )
        {
            under_test.retain( publish_non_zero );
            under_test.retain( publish_zero );

            THEN( "retained_messages#size() should return 0" )
            {
                REQUIRE( under_test.size( ) == 0 );
            }
        }
    }
}
