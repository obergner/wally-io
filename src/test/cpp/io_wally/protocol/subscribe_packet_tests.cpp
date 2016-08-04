#include "catch.hpp"

#include "io_wally/protocol/suback_packet.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"

using namespace io_wally::protocol;

SCENARIO( "subscribe", "[packets]" )
{
    GIVEN( "a subscribe packet with packet identifier and 3 subscriptions" )
    {
        auto header = packet::header{0x08 << 4, 20};  // 20 is just some number
        auto packet_id = uint16_t{9};
        auto subscriptions = std::vector<subscription>{{"/topic/*/1", packet::QoS::AT_MOST_ONCE},
                                                       {"/topic/*/2", packet::QoS::AT_LEAST_ONCE},
                                                       {"/topic/*/3", packet::QoS::EXACTLY_ONCE}};
        auto under_test = subscribe{header, packet_id, subscriptions};

        WHEN( "a caller asks for the packet identifier" )
        {
            auto packet_id_ret = under_test.packet_identifier( );

            THEN( "it should see correct packet identifier returned" )
            {
                REQUIRE( packet_id_ret == packet_id );
            }
        }

        WHEN( "a caller asks for subscribe packet's subscriptions" )
        {
            auto subscriptions_ret = under_test.subscriptions( );

            THEN( "it should see correct subscriptions returned" )
            {
                REQUIRE( subscriptions_ret == subscriptions );
            }
        }

        WHEN( "a caller asks for a failure suback" )
        {
            auto expected_rcs = std::vector<suback_return_code>{3, suback_return_code::FAILURE};
            auto failure_suback = under_test.fail( );

            THEN( "it should see correct failure suback returned" )
            {
                CHECK( failure_suback->packet_identifier( ) == packet_id );
                REQUIRE( failure_suback->return_codes( ) == expected_rcs );
            }
        }

        WHEN( "a caller asks for a success suback" )
        {
            auto expected_rcs = std::vector<suback_return_code>{
                suback_return_code::MAXIMUM_QOS0, suback_return_code::MAXIMUM_QOS1, suback_return_code::MAXIMUM_QOS2};
            auto success_suback = under_test.succeed( );

            THEN( "it should see correct success suback returned" )
            {
                CHECK( success_suback->packet_identifier( ) == packet_id );
                REQUIRE( success_suback->return_codes( ) == expected_rcs );
            }
        }
    }
}
