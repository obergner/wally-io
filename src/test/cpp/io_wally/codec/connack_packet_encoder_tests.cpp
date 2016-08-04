#include "catch.hpp"

#include <array>
#include <cstdint>

#include "io_wally/codec/connack_packet_encoder.hpp"

using namespace io_wally;

using out_iter = std::array<const std::uint8_t, 2>::iterator;

SCENARIO( "connack_packet_encoder", "[encoder]" )
{
    encoder::connack_packet_encoder<std::uint8_t*> under_test;

    GIVEN( "a connack packet with session present flag set and return code 'CONNECTION_ACCEPTED'" )
    {
        const protocol::connack connack( true, protocol::connect_return_code::CONNECTION_ACCEPTED );

        std::array<std::uint8_t, 2> result = {{0x00, 0x00}};
        const std::array<std::uint8_t, 2> expected_result = {{0x01, 0x00}};

        WHEN( "a client passes that packet into connack_packet_encoder::encode" )
        {
            out_iter new_buf_start = under_test.encode( connack, result.begin( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == connack.header( ).remaining_length( ) );
            }
        }
    }

    GIVEN( "a connack packet with session present flag not set and return code 'IDENTIFIER_REJECTED'" )
    {
        const protocol::connack connack( false, protocol::connect_return_code::IDENTIFIER_REJECTED );

        std::array<std::uint8_t, 2> result = {{0xFF, 0xFF}};
        const std::array<std::uint8_t, 2> expected_result = {{0x00, 0x02}};

        WHEN( "a client passes that packet into connack_packet_encoder::encode" )
        {
            out_iter new_buf_start = under_test.encode( connack, result.begin( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == connack.header( ).remaining_length( ) );
            }
        }
    }
}
