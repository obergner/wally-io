#include "catch.hpp"

#include "io_wally/codec/connack_packet_encoder.hpp"

using namespace io_wally;

typedef std::array<const uint8_t, 2>::iterator out_iter;

SCENARIO( "connack_packet_encoder", "[encoder]" )
{
    encoder::connack_packet_encoder<uint8_t*> under_test;

    GIVEN( "a connack packet with session present flag set and return code 'CONNECTION_ACCEPTED'" )
    {
        const connack connack( true, connect_return_code::CONNECTION_ACCEPTED );

        std::array<uint8_t, 2> result = {{0x00, 0x00}};
        const std::array<uint8_t, 2> expected_result = {{0x01, 0x00}};

        WHEN( "a client passes that packet into connack_packet_encoder::encode" )
        {
            out_iter new_buf_start = under_test.encode( connack, result.begin( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == 2 );
            }
        }
    }
}
