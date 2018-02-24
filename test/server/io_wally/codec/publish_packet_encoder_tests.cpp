#include "catch.hpp"

#include <array>
#include <cstdint>
#include <vector>

#include "io_wally/codec/publish_packet_encoder.hpp"
#include "io_wally/protocol/publish_packet.hpp"

using namespace io_wally;

// Yet another act of unabashed robbery: https://github.com/surgemq/message/blob/master/publish_test.go

SCENARIO( "publish_packet_encoder", "[encoder]" )
{
    const auto under_test = encoder::publish_packet_encoder<std::uint8_t*>{};

    GIVEN( "a PUBLISH packet with QoS != 0, i.e. WITH packet identifier" )
    {
        const auto type_and_flags = uint8_t{3 << 4 | 0x02};  // QoS 1
        const auto remaining_length = uint32_t{23};
        const auto packet_identifier = uint16_t{7};
        const auto topic = "surgemq";
        const auto application_message =
            std::vector<uint8_t>{'s', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e'};
        const auto publish =
            protocol::publish{type_and_flags, remaining_length, topic, packet_identifier, application_message};

        auto result =
            std::array<std::uint8_t, 23>{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
        const auto expected_result = std::array<std::uint8_t, 23>{{
            0x00, 0x07, 's', 'u', 'r', 'g', 'e', 'm', 'q', 0x00, 0x07, 's',
            'e',  'n',  'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm',  'e',
        }};

        WHEN( "a client passes that packet into publish_packet_encoder::encode" )
        {
            const auto new_buf_start = under_test.encode( publish, result.begin( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == publish.remaining_length( ) );
            }
        }
    }

    GIVEN( "a PUBLISH packet with QoS = 0, i.e. WITHOUT packet identifier" )
    {
        const auto type_and_flags = uint8_t{3 << 4 | 0x00};  // QoS 0
        const auto remaining_length = uint32_t{21};
        const auto packet_identifier = uint16_t{0};
        const auto topic = "surgemq";
        const auto application_message =
            std::vector<uint8_t>{'s', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e'};
        const auto publish =
            protocol::publish{type_and_flags, remaining_length, topic, packet_identifier, application_message};

        auto result = std::array<std::uint8_t, 21>{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
        const auto expected_result = std::array<std::uint8_t, 21>{{
            0x00, 0x07, 's', 'u', 'r', 'g', 'e', 'm', 'q', 's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e',
        }};

        WHEN( "a client passes that packet into publish_packet_encoder::encode" )
        {
            const auto new_buf_start = under_test.encode( publish, result.begin( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == publish.remaining_length( ) );
            }
        }
    }
}
