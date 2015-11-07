#include "catch.hpp"

#include <cstdint>
#include <array>
#include <vector>

#include "io_wally/protocol/pubrel_packet.hpp"
#include "io_wally/codec/pubrel_packet_encoder.hpp"

using namespace io_wally;

SCENARIO( "pubrel_packet_encoder", "[encoder]" )
{
    auto under_test = encoder::pubrel_packet_encoder<std::uint8_t*>{};

    GIVEN( "a pubrel packet" )
    {
        auto packet_identifier = uint16_t{7};
        auto pubrel = protocol::pubrel{packet_identifier};

        auto result = std::array<std::uint8_t, 2>{{0x00, 0x00}};
        auto expected_result = std::array<std::uint8_t, 2>{{0x00, 0x07}};

        WHEN( "a client passes that packet into pubrel_packet_encoder::encode" )
        {
            auto new_buf_start = under_test.encode( pubrel, result.begin( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == pubrel.header( ).remaining_length( ) );
            }
        }
    }
}
