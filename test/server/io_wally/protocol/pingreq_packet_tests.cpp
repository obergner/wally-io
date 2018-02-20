#include "catch.hpp"

#include <cstdint>

#include "io_wally/protocol/pingreq_packet.hpp"

using namespace io_wally::protocol;

SCENARIO( "pingreq packet", "[packets]" )
{

    GIVEN( "a pingreq packet instance" )
    {
        const pingreq under_test;

        WHEN( "a caller asks for its packet type" )
        {
            const packet::Type packet_type = under_test.type( );

            THEN( "it should see PINGREQ" )
            {
                REQUIRE( packet_type == packet::Type::PINGREQ );
            }
        }
    }

    GIVEN( "a pingreq packet instance" )
    {
        const pingreq under_test;

        WHEN( "a caller asks for its remaining_length" )
        {
            const std::uint32_t remaining_length = under_test.header( ).remaining_length( );

            THEN( "it should see 0" )
            {
                REQUIRE( remaining_length == 0 );
            }
        }
    }

    GIVEN( "a pingreq packet instance" )
    {
        const pingreq under_test;

        WHEN( "a caller asks for its total_length" )
        {
            const std::uint32_t total_length = under_test.header( ).total_length( );

            THEN( "it should see 2" )
            {
                REQUIRE( total_length == 2 );
            }
        }
    }
}
