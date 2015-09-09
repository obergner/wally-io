#include "catch.hpp"

#include "io_wally/codec/mqtt_packet_encoder.hpp"

using namespace io_wally;

typedef std::array<const uint8_t, 4>::iterator out_iter;

SCENARIO( "mqtt_packet_encoder", "[encoder]" )
{
    encoder::mqtt_packet_encoder<uint8_t*> under_test;

    GIVEN( "an output buffer with insufficient capacity 3" )
    {
        const connack connack( true, connect_return_code::NOT_AUTHORIZED );

        std::array<uint8_t, 3> result = {{0x00, 0x00, 0x00}};

        WHEN( "a client passes that packet into mqtt_packet_encoder::encode" )
        {
            THEN( "that client should see a std::invalid_argument exception being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.encode( connack, result.begin( ), result.end( ) ),
                                   std::invalid_argument );
            }
        }
    }

    GIVEN( "a connack packet with session present flag set and return code 'NOT_AUTHORIZED'" )
    {
        const connack connack( true, connect_return_code::NOT_AUTHORIZED );

        std::array<uint8_t, 4> result = {{0x00, 0x00, 0x00, 0x00}};
        const std::array<uint8_t, 4> expected_result = {{0x20, 0x02, 0x01, 0x05}};

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
        const pingresp pingresp;

        std::array<uint8_t, 2> result = {{0x00, 0x00}};
        const std::array<uint8_t, 2> expected_result = {{0xD0, 0x00}};

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
