#include "catch.hpp"

#include "io_wally/protocol/common.hpp"
#include "io_wally/dispatch/common.hpp"
#include "io_wally/dispatch/topic_subscriptions.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/protocol/publish_packet.hpp"

using namespace io_wally::protocol;

// SCENARIO( "topic_subscriptions#resolve_subscribers", "[dispatch]" )
//{
//    static const packet::QoS MAX_QOS = packet::QoS::AT_LEAST_ONCE;
//    static const std::string CLIENT_ID = "subscription_container_tests";
//
//    GIVEN(
//        "topic filter \"/sports/premiere-league/barcelona/\" and the same topic "
//        "\"/sports/premiere-league/barcelona/\"" )
//    {
//        auto header = packet::header{0x08 << 4, 20};  // 20 is just some number
//        auto packet_id = uint16_t{9};
//        auto subscriptions = std::vector<subscription>{{"/topic/*/level", packet::QoS::AT_MOST_ONCE},
//                                                       {"/topic/*/*", packet::QoS::AT_LEAST_ONCE},
//                                                       {"/topic/#", packet::QoS::EXACTLY_ONCE}};
//        auto subscribe_packet = subscribe{header, packet_id, subscriptions};
//
//        auto const topic_filter = "/sports/premiere-league/barcelona/";
//        auto const topic = "/sports/premiere-league/barcelona/";
//        auto const under_test = io_wally::dispatch::subscription_container{topic_filter, MAX_QOS, CLIENT_ID};
//
//        WHEN( "a caller matches topic against topic filter " )
//        {
//            THEN( "it should see the match succeed" )
//            {
//                REQUIRE( under_test.matches( topic ) == true );
//            }
//        }
//    }
//}
