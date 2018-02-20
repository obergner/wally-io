#include "catch.hpp"

#include <array>
#include <cstdint>
#include <vector>

#include "io_wally/codec/pubcomp_packet_encoder.hpp"
#include "io_wally/protocol/pubcomp_packet.hpp"

using namespace io_wally;

SCENARIO( "pubcomp_packet_encoder", "[encoder]" )
{
    auto under_test = encoder::pubcomp_packet_encoder<std::uint8_t*>{};

    GIVEN( "a pubcomp packet" )
    {
        auto packet_identifier = uint16_t{65535};
        auto pubcomp = protocol::pubcomp{packet_identifier};

        auto result = std::array<std::uint8_t, 2>{{0x00, 0x00}};
        auto expected_result = std::array<std::uint8_t, 2>{{0xFF, 0xFF}};

        WHEN( "a client passes that packet into pubcomp_packet_encoder::encode" )
        {
            auto new_buf_start = under_test.encode( pubcomp, result.begin( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == pubcomp.remaining_length( ) );
            }
        }
    }
}
