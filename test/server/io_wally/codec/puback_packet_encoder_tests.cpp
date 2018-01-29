#include "catch.hpp"

#include <array>
#include <cstdint>
#include <vector>

#include "io_wally/codec/puback_packet_encoder.hpp"
#include "io_wally/protocol/puback_packet.hpp"

using namespace io_wally;

SCENARIO( "puback_packet_encoder", "[encoder]" )
{
    auto under_test = encoder::puback_packet_encoder<std::uint8_t*>{};

    GIVEN( "a puback packet" )
    {
        auto packet_identifier = uint16_t{65535};
        auto puback = protocol::puback{packet_identifier};

        auto result = std::array<std::uint8_t, 2>{{0x00, 0x00}};
        auto expected_result = std::array<std::uint8_t, 2>{{0xFF, 0xFF}};

        WHEN( "a client passes that packet into puback_packet_encoder::encode" )
        {
            auto new_buf_start = under_test.encode( puback, result.begin( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == puback.header( ).remaining_length( ) );
            }
        }
    }
}
