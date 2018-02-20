#include "catch.hpp"

#include <cstdint>

#include "io_wally/protocol/pingresp_packet.hpp"

using namespace io_wally::protocol;

SCENARIO( "pingresp packet", "[packets]" )
{

    GIVEN( "a pingresp packet instance" )
    {
        const pingresp under_test;

        WHEN( "a caller asks for its packet type" )
        {
            const packet::Type packet_type = under_test.type( );

            THEN( "it should see PINGRESP" )
            {
                REQUIRE( packet_type == packet::Type::PINGRESP );
            }
        }
    }

    GIVEN( "a pingresp packet instance" )
    {
        const pingresp under_test;

        WHEN( "a caller asks for its remaining_length" )
        {
            const std::uint32_t remaining_length = under_test.header( ).remaining_length( );

            THEN( "it should see 0" )
            {
                REQUIRE( remaining_length == 0 );
            }
        }
    }

    GIVEN( "a pingresp packet instance" )
    {
        const pingresp under_test;

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
