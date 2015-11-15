#include "catch.hpp"

#include <cstdint>
#include <array>
#include <vector>

#include "io_wally/protocol/pubrec_packet.hpp"
#include "io_wally/codec/pubrec_packet_encoder.hpp"

using namespace io_wally;

SCENARIO( "pubrec_packet_encoder", "[encoder]" )
{
    auto under_test = encoder::pubrec_packet_encoder<std::uint8_t*>{};

    GIVEN( "a pubrec packet" )
    {
        auto packet_identifier = uint16_t{65535};
        auto pubrec = protocol::pubrec{packet_identifier};

        auto result = std::array<std::uint8_t, 2>{{0x00, 0x00}};
        auto expected_result = std::array<std::uint8_t, 2>{{0xFF, 0xFF}};

        WHEN( "a client passes that packet into pubrec_packet_encoder::encode" )
        {
            auto new_buf_start = under_test.encode( pubrec, result.begin( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == pubrec.header( ).remaining_length( ) );
            }
        }
    }
}
