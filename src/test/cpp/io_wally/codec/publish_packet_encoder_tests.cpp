#include "catch.hpp"

#include <cstdint>
#include <array>
#include <vector>

#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/codec/publish_packet_encoder.hpp"

using namespace io_wally;

// Yet another act of unabashed robbery: https://github.com/surgemq/message/blob/master/publish_test.go

SCENARIO( "publish_packet_encoder", "[encoder]" )
{
    auto under_test = encoder::publish_packet_encoder<std::uint8_t*>{};

    GIVEN( "a PUBLISH packet with QoS != 0, i.e. WITH packet identifier" )
    {
        auto header = protocol::packet::header{3 << 4 | 0x02, 23};  // QoS 1
        auto packet_identifier = uint16_t{7};
        auto topic = "surgemq";
        auto application_message = std::vector<uint8_t>{'s', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e'};
        auto publish = protocol::publish{header, topic, packet_identifier, application_message};

        auto result =
            std::array<std::uint8_t, 23>{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
        auto expected_result = std::array<std::uint8_t, 23>{{
            0x00, 0x07, 's', 'u', 'r', 'g', 'e', 'm', 'q', 0x00, 0x07, 's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o',
            'm', 'e',
        }};

        WHEN( "a client passes that packet into publish_packet_encoder::encode" )
        {
            auto new_buf_start = under_test.encode( publish, result.begin( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == publish.header( ).remaining_length( ) );
            }
        }
    }

    GIVEN( "a PUBLISH packet with QoS = 0, i.e. WITHOUT packet identifier" )
    {
        auto header = protocol::packet::header{3 << 4 | 0x00, 21};
        auto packet_identifier = uint16_t{0};
        auto topic = "surgemq";
        auto application_message = std::vector<uint8_t>{'s', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e'};
        auto publish = protocol::publish{header, topic, packet_identifier, application_message};

        auto result = std::array<std::uint8_t, 21>{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
        auto expected_result = std::array<std::uint8_t, 21>{{
            0x00, 0x07, 's', 'u', 'r', 'g', 'e', 'm', 'q', 's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e',
        }};

        WHEN( "a client passes that packet into publish_packet_encoder::encode" )
        {
            auto new_buf_start = under_test.encode( publish, result.begin( ) );

            THEN( "that client should see a correctly encoded buffer" )
            {
                REQUIRE( result == expected_result );
            }

            AND_THEN( "it should see a correctly advanced out iterator" )
            {
                REQUIRE( ( new_buf_start - result.begin( ) ) == publish.header( ).remaining_length( ) );
            }
        }
    }
}
