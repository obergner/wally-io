#include "catch.hpp"

#include <cstdint>
#include <array>
#include <vector>

#include "io_wally/protocol/suback_packet.hpp"
#include "io_wally/codec/suback_packet_encoder.hpp"

using namespace io_wally;

using out_iter = std::array<const std::uint8_t, 2>::iterator;

SCENARIO( "suback_packet_encoder", "[encoder]" )
{
    encoder::suback_packet_encoder<std::uint8_t*> under_test;

    GIVEN( "a suback packet with 3 return codes" )
    {
        const uint16_t packet_identifier = 7;
        const std::vector<protocol::suback_return_code> return_codes = {{protocol::suback_return_code::MAXIMUM_QOS1,
                                                                         protocol::suback_return_code::MAXIMUM_QOS2,
                                                                         protocol::suback_return_code::FAILURE}};
        const protocol::suback suback{packet_identifier, return_codes};

        std::array<std::uint8_t, 5> result = {{0x00, 0x00, 0x00, 0x00, 0x00}};
        const std::array<std::uint8_t, 5> expected_result = {{0x00, 0x07, 0x01, 0x02, 0x80}};

        WHEN( "a client passes that packet into suback_packet_encoder::encode" )
        {
            out_iter new_buf_start = under_test.encode( suback, result.begin( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == suback.header( ).remaining_length( ) );
            }
        }
    }
}
