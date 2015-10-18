#include "catch.hpp"

#include <cstdint>
#include <array>

#include "io_wally/codec/mqtt_packet_encoder.hpp"

using namespace io_wally;

using out_iter = std::array<const std::uint8_t, 4>::iterator;

SCENARIO( "mqtt_packet_encoder", "[encoder]" )
{
    encoder::mqtt_packet_encoder<std::uint8_t*> under_test;

    GIVEN( "a connack packet with session present flag set and return code 'NOT_AUTHORIZED'" )
    {
        const protocol::connack connack( true, protocol::connect_return_code::NOT_AUTHORIZED );

        std::array<std::uint8_t, 4> result = {{0x00, 0x00, 0x00, 0x00}};
        const std::array<std::uint8_t, 4> expected_result = {{0x20, 0x02, 0x01, 0x05}};

        WHEN( "a client passes that packet into mqtt_packet_encoder::encode" )
        {
            out_iter new_buf_start = under_test.encode( connack, result.begin( ), result.end( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == connack.header( ).total_length( ) );
            }
        }
    }

    GIVEN( "a pingresp packet" )
    {
        const protocol::pingresp pingresp;

        std::array<std::uint8_t, 2> result = {{0x00, 0x00}};
        const std::array<std::uint8_t, 2> expected_result = {{0xD0, 0x00}};

        WHEN( "a client passes that packet into mqtt_packet_encoder::encode" )
        {
            out_iter new_buf_start = under_test.encode( pingresp, result.begin( ), result.end( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == pingresp.header( ).total_length( ) );
            }
        }
    }
}
