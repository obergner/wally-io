#include "catch.hpp"

#include <array>
#include <cstdint>

#include "io_wally/codec/unsuback_packet_encoder.hpp"
#include "io_wally/protocol/unsuback_packet.hpp"

using namespace io_wally;

SCENARIO( "unsuback_packet_encoder", "[encoder]" )
{
    const auto under_test = encoder::unsuback_packet_encoder<std::uint8_t*>{};

    GIVEN( "an unsuback packet" )
    {
        const auto packet_identifier = uint16_t{65535};
        const auto unsuback = protocol::unsuback{packet_identifier};

        auto result = std::array<std::uint8_t, 2>{{0x00, 0x00}};
        const auto expected_result = std::array<std::uint8_t, 2>{{0xFF, 0xFF}};

        WHEN( "a client passes that packet into unsuback_packet_encoder::encode" )
        {
            const auto new_buf_start = under_test.encode( unsuback, result.begin( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == unsuback.remaining_length( ) );
            }
        }
    }
}
