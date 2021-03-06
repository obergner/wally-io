#include "catch.hpp"

#include <array>
#include <cstdint>

#include "io_wally/codec/mqtt_packet_encoder.hpp"
#include "io_wally/protocol/protocol.hpp"

using namespace io_wally;

using out_iter = std::array<const std::uint8_t, 4>::iterator;

SCENARIO( "mqtt_packet_encoder", "[encoder]" )
{
    encoder::mqtt_packet_encoder<std::uint8_t*> under_test;

    GIVEN( "a connack packet with session present flag set and return code 'NOT_AUTHORIZED'" )
    {
        const protocol::connack connack{true, protocol::connect_return_code::NOT_AUTHORIZED};

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
                REQUIRE( ( new_buf_start - result.begin( ) ) == connack.total_length( ) );
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
                REQUIRE( ( new_buf_start - result.begin( ) ) == pingresp.total_length( ) );
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

        WHEN( "a client passes that packet into mqtt_packet_encoder::encode" )
        {
            auto new_buf_start = under_test.encode( suback, result.begin( ), result.end( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == suback.total_length( ) );
            }
        }
    }

    GIVEN( "a PUBLISH packet with QoS 1" )
    {
        const auto type_and_flags = uint8_t{3 << 4 | 0x02};
        const auto remaining_length = uint32_t{23};
        const auto packet_identifier = uint16_t{7};
        const auto topic = "surgemq";
        auto application_message = std::vector<uint8_t>{'s', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e'};
        const auto publish =
            protocol::publish{type_and_flags, remaining_length, topic, packet_identifier, application_message};

        auto result =
            std::array<std::uint8_t, 25>{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
        const auto expected_result = std::array<std::uint8_t, 25>{{
            ( 3 << 4 | 0x02 ),
            0x17,
            0x00,
            0x07,
            's',
            'u',
            'r',
            'g',
            'e',
            'm',
            'q',
            0x00,
            0x07,
            's',
            'e',
            'n',
            'd',
            ' ',
            'm',
            'e',
            ' ',
            'h',
            'o',
            'm',
            'e',
        }};

        WHEN( "a client passes that packet into mqtt_packet_encoder::encode" )
        {
            auto new_buf_start = under_test.encode( publish, result.begin( ), result.end( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == publish.total_length( ) );
            }
        }
    }

    GIVEN( "a puback packet" )
    {
        auto packet_identifier = uint16_t{65535};
        auto puback = protocol::puback{packet_identifier};

        auto result = std::array<std::uint8_t, 4>{{0x00, 0x00, 0x00, 0x00}};
        auto expected_result = std::array<std::uint8_t, 4>{{0x40, 0x02, 0xFF, 0xFF}};

        WHEN( "a client passes that packet into mqtt_packet_encoder::encode" )
        {
            auto new_buf_start = under_test.encode( puback, result.begin( ), result.end( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == puback.total_length( ) );
            }
        }
    }

    GIVEN( "a pubrel packet" )
    {
        auto packet_identifier = uint16_t{7};
        auto pubrel = protocol::pubrel{packet_identifier};

        auto result = std::array<std::uint8_t, 4>{{0x00, 0x00, 0x00, 0x00}};
        auto expected_result = std::array<std::uint8_t, 4>{{0x62, 0x02, 0x00, 0x07}};

        WHEN( "a client passes that packet into mqtt_packet_encoder::encode" )
        {
            auto new_buf_start = under_test.encode( pubrel, result.begin( ), result.end( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == pubrel.total_length( ) );
            }
        }
    }

    GIVEN( "a pubrec packet" )
    {
        auto packet_identifier = uint16_t{65535};
        auto pubrec = protocol::pubrec{packet_identifier};

        auto result = std::array<std::uint8_t, 4>{{0x00, 0x00, 0x00, 0x00}};
        auto expected_result = std::array<std::uint8_t, 4>{{0x50, 0x02, 0xFF, 0xFF}};

        WHEN( "a client passes that packet into pubrec_packet_encoder::encode" )
        {
            auto new_buf_start = under_test.encode( pubrec, result.begin( ), result.end( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == pubrec.total_length( ) );
            }
        }
    }

    GIVEN( "a pubcomp packet" )
    {
        auto packet_identifier = uint16_t{65535};
        auto pubcomp = protocol::pubcomp{packet_identifier};

        auto result = std::array<std::uint8_t, 4>{{0x00, 0x00, 0x00, 0x00}};
        auto expected_result = std::array<std::uint8_t, 4>{{0x70, 0x02, 0xFF, 0xFF}};

        WHEN( "a client passes that packet into pubcomp_packet_encoder::encode" )
        {
            auto new_buf_start = under_test.encode( pubcomp, result.begin( ), result.end( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == pubcomp.total_length( ) );
            }
        }
    }

    GIVEN( "an unsuback packet" )
    {
        auto packet_identifier = uint16_t{65535};
        auto unsuback = protocol::unsuback{packet_identifier};

        auto result = std::array<std::uint8_t, 4>{{0x00, 0x00, 0x00, 0x00}};
        auto expected_result = std::array<std::uint8_t, 4>{{0xB0, 0x02, 0xFF, 0xFF}};

        WHEN( "a client passes that packet into mqtt_packet_encoder::encode" )
        {
            auto new_buf_start = under_test.encode( unsuback, result.begin( ), result.end( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == unsuback.total_length( ) );
            }
        }
    }
}
