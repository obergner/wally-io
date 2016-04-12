#include "catch.hpp"

#include "framework/factories.hpp"

#include "io_wally/protocol/common.hpp"
#include "io_wally/dispatch/common.hpp"
#include "io_wally/dispatch/topic_subscriptions.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/protocol/unsubscribe_packet.hpp"
#include "io_wally/protocol/publish_packet.hpp"

using namespace io_wally::protocol;

SCENARIO( "topic_subscriptions#resolve_subscribers", "[dispatch]" )
{
    GIVEN( "topic_subscriptions with three subscriptions having different QoS" )
    {
        io_wally::dispatch::topic_subscriptions under_test{};

        auto const client_id = "topic_subscription_tests";
        auto subscriptions = std::vector<subscription>{{"/topic/+/level", packet::QoS::AT_MOST_ONCE},
                                                       {"/topic/+/+", packet::QoS::AT_LEAST_ONCE},
                                                       {"/topic/#", packet::QoS::EXACTLY_ONCE}};
        auto subscr_cont = framework::create_subscribe_packet( subscriptions );
        under_test.subscribe( client_id, subscr_cont );

        WHEN( "a caller resolves subscribers for a topic that matches all three subscriptions " )
        {
            auto publish_cont = framework::create_publish_packet( "/topic/first/level" );
            auto subscribers = under_test.resolve_subscribers( publish_cont );

            THEN( "it should receive a single subscriber" )
            {
                REQUIRE( subscribers.size( ) == 1 );
            }

            AND_THEN( "that subscriber should have the maximum of all three QoS levels assigned" )
            {
                auto const assigned_qos = subscribers[0].second;
                REQUIRE( assigned_qos == packet::QoS::EXACTLY_ONCE );
            }
        }
    }
}

SCENARIO( "topic_subscriptions#unsubscribe", "[dispatch]" )
{
    GIVEN( "topic_subscriptions with three subscriptions" )
    {
        io_wally::dispatch::topic_subscriptions under_test{};

        auto const client_id = "topic_subscription_tests";
        auto subscriptions = std::vector<subscription>{{"/first/+/level", packet::QoS::AT_MOST_ONCE},
                                                       {"/topic/+/+", packet::QoS::AT_LEAST_ONCE},
                                                       {"/topic/#", packet::QoS::EXACTLY_ONCE}};
        auto subscr_cont = framework::create_subscribe_packet( subscriptions );
        under_test.subscribe( client_id, subscr_cont );

        WHEN( "a caller unsubscribes from one of the subscriptions and publishes to that subscriptions" )
        {
            auto const unsubscribe = framework::create_unsubscribe_packet( {"/first/+/level"} );
            under_test.unsubscribe( client_id, unsubscribe );

            auto publish_cont = framework::create_publish_packet( "/first/ignored/level" );
            auto subscribers = under_test.resolve_subscribers( publish_cont );

            THEN( "it should receive no subscriber" )
            {
                REQUIRE( subscribers.size( ) == 0 );
            }
        }
    }
}
