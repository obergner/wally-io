#include "catch.hpp"

#include <array>
#include <cstdint>

#include "io_wally/codec/pingresp_packet_encoder.hpp"

using namespace io_wally;

typedef std::array<const std::uint8_t, 0>::iterator out_iter;

SCENARIO( "pingresp_packet_encoder", "[encoder]" )
{
    encoder::pingresp_packet_encoder<std::uint8_t*> under_test;

    GIVEN( "a pingresp packet" )
    {
        const protocol::pingresp pingresp;

        std::array<std::uint8_t, 0> result = {{}};
        const std::array<std::uint8_t, 0> expected_result = {{}};

        WHEN( "a client passes that packet into pingresp_packet_encoder::encode" )
        {
            out_iter new_buf_start = under_test.encode( pingresp, result.begin( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == pingresp.remaining_length( ) );
            }
        }
    }
}
