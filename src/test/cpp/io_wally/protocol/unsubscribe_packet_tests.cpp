#include "catch.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include "io_wally/protocol/unsubscribe_packet.hpp"
#include "io_wally/protocol/unsuback_packet.hpp"

using namespace io_wally::protocol;

SCENARIO( "unsubscribe", "[packets]" )
{
    GIVEN( "a unsubscribe packet with packet identifier and 3 subscriptions" )
    {
        const auto header = packet::header{0x0A << 4, 20};  // 20 is just some number
        const auto packet_id = uint16_t{9};
        const auto topic_filters = std::vector<std::string>{"/topic/*/1", "/topic/*/2", "/topic/*/3"};
        const auto under_test = unsubscribe{header, packet_id, topic_filters};

        WHEN( "a caller asks for the packet identifier" )
        {
            const auto packet_id_ret = under_test.packet_identifier( );

            THEN( "it should see correct packet identifier returned" )
            {
                REQUIRE( packet_id_ret == packet_id );
            }
        }

        WHEN( "a caller asks for the topic filters" )
        {
            const auto topic_filters_ret = under_test.topic_filters( );

            THEN( "it should see correct topic filters returned" )
            {
                REQUIRE( topic_filters_ret == topic_filters );
            }
        }

        WHEN( "a caller asks for an unsuback" )
        {
            const auto unsuback = under_test.ack( );

            THEN( "it should see correct failure unsuback returned" )
            {
                REQUIRE( unsuback->packet_identifier( ) == packet_id );
            }
        }
    }
}
