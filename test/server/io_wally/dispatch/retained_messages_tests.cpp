#include "catch.hpp"

#include "framework/factories.hpp"

#include "io_wally/dispatch/common.hpp"
#include "io_wally/dispatch/retained_messages.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_packet.hpp"

using namespace io_wally::protocol;

SCENARIO( "retained_messages#retain", "[dispatch]" )
{
    GIVEN( "a PUBLISH packet with retain flag set" )
    {
        io_wally::dispatch::retained_messages under_test{};

        const auto topic = "/test/retain";
        const auto publish = framework::create_publish_packet( topic, true );

        WHEN( "no messages have been retained yet" )
        {
            THEN( "retained_messages#size() should return 0" )
            {
                REQUIRE( under_test.size( ) == 0 );
            }
        }

        WHEN( "a caller stores this PUBLISH packet in retained_messages " )
        {
            under_test.retain( publish );

            THEN( "retained_messages#size() should return 1" )
            {
                REQUIRE( under_test.size( ) == 1 );
            }
        }

        WHEN( "a caller stores this PUBLISH packet in retained_messages twice" )
        {
            under_test.retain( publish );
            under_test.retain( publish );

            THEN( "retained_messages#size() should still return 1" )
            {
                REQUIRE( under_test.size( ) == 1 );
            }
        }
    }

    GIVEN( "a PUBLISH packet with retain flag set and zero-length payload" )
    {
        io_wally::dispatch::retained_messages under_test{};

        const auto topic = "/test/retain";
        const auto publish_non_zero =
            framework::create_publish_packet( topic, true, std::vector<uint8_t>{'n', 'o', 'n', 'z', 'e', 'r', 'o'} );
        const auto publish_zero = framework::create_publish_packet( topic, true, std::vector<uint8_t>{} );

        WHEN(
            "a caller first stores a non-zero-length PUBLISH packet and then a zero-length PUBLISH packet for the same "
            "topic" )
        {
            under_test.retain( publish_non_zero );
            under_test.retain( publish_zero );

            THEN( "retained_messages#size() should return 0" )
            {
                REQUIRE( under_test.size( ) == 0 );
            }
        }
    }
}

SCENARIO( "retained_messages#messages_for", "[dispatch]" )
{
    GIVEN( "a retained_messages instance containing 5 retained messages" )
    {
        io_wally::dispatch::retained_messages under_test{};

        under_test.retain( framework::create_publish_packet( "/test/retain1/topic1", true ) );
        under_test.retain( framework::create_publish_packet( "/test/retain2/topic2", true ) );
        under_test.retain( framework::create_publish_packet( "/test/retain3/topic3", true ) );
        under_test.retain( framework::create_publish_packet( "/test/retain4/topic4", true ) );
        under_test.retain( framework::create_publish_packet( "/test/retain4/topic5", true ) );

        WHEN( "a caller calls messages_for( subscribe ) with a subscribe packet that matches 3 messages" )
        {
            auto subscribe = framework::create_subscribe_packet(
                {{"/test/retain1/topic1", io_wally::protocol::packet::QoS::AT_MOST_ONCE},
                 {"/test/retain4/+", io_wally::protocol::packet::QoS::EXACTLY_ONCE}} );
            const auto matches = under_test.messages_for( subscribe );

            THEN( "retained_messages#messages_for() should return all 3 matching messages" )
            {
                REQUIRE( matches.size( ) == 3 );
            }
        }
    }
}
