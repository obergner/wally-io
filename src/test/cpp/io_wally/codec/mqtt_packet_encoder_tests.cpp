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

    GIVEN( "a suback packet with 3 return codes" )
    {
        auto packet_identifier = uint16_t{7};
        auto return_codes = std::vector<protocol::suback_return_code>{protocol::suback_return_code::MAXIMUM_QOS1,
                                                                      protocol::suback_return_code::MAXIMUM_QOS2,
                                                                      protocol::suback_return_code::FAILURE};
        auto suback = protocol::suback{packet_identifier, return_codes};

        auto result = std::array<std::uint8_t, 7>{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
        auto expected_result = std::array<std::uint8_t, 7>{{0x90, 0x05, 0x00, 0x07, 0x01, 0x02, 0x80}};

        WHEN( "a client passes that packet into suback_packet_encoder::encode" )
        {
            auto new_buf_start = under_test.encode( suback, result.begin( ), result.end( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == suback.header( ).total_length( ) );
            }
        }
    }
}