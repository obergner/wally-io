#include "catch.hpp"

#include <array>
#include <cstdint>
#include <vector>

#include "io_wally/codec/suback_packet_encoder.hpp"
#include "io_wally/protocol/suback_packet.hpp"

using namespace io_wally;

SCENARIO( "suback_packet_encoder", "[encoder]" )
{
    const auto under_test = encoder::suback_packet_encoder<std::uint8_t*>{};

    GIVEN( "a suback packet with 3 return codes" )
    {
        const auto packet_identifier = uint16_t{7};
        auto return_codes = std::vector<protocol::suback_return_code>{protocol::suback_return_code::MAXIMUM_QOS1,
                                                                      protocol::suback_return_code::MAXIMUM_QOS2,
                                                                      protocol::suback_return_code::FAILURE};
        const auto suback = protocol::suback{packet_identifier, return_codes};

        auto result = std::array<std::uint8_t, 5>{{0x00, 0x00, 0x00, 0x00, 0x00}};
        const auto expected_result = std::array<std::uint8_t, 5>{{0x00, 0x07, 0x01, 0x02, 0x80}};

        WHEN( "a client passes that packet into suback_packet_encoder::encode" )
        {
            const auto new_buf_start = under_test.encode( suback, result.begin( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == suback.remaining_length( ) );
            }
        }
    }
}
