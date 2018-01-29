#include "catch.hpp"

#include "io_wally/error/protocol.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/subscription.hpp"

using namespace io_wally::protocol;

namespace
{
    const std::string random_string( size_t length )
    {
        auto randchar = []( ) -> char {
            const char charset[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = ( sizeof( charset ) - 1 );
            return charset[rand( ) % max_index];
        };
        std::string str( length, 0 );
        std::generate_n( str.begin( ), length, randchar );
        return str;
    }
}

SCENARIO( "subscription", "[packets]" )
{
    GIVEN( "topic filter \"\"" )
    {
        auto const topic_filter = "";
        WHEN( "a caller tries to create a subscription" )
        {
            THEN( "it should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( subscription( topic_filter, packet::QoS::AT_LEAST_ONCE ),
                                   io_wally::error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a well-formed topic filter of length 65535" )
    {
        auto const topic_filter = random_string( 65535 );
        WHEN( "a caller tries to create a subscription" )
        {
            THEN( "it should NOT see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_NOTHROW( subscription( topic_filter, packet::QoS::AT_LEAST_ONCE ) );
            }
        }
    }

    GIVEN( "a topic filter of length 65536" )
    {
        auto const topic_filter = random_string( 65536 );
        WHEN( "a caller tries to create a subscription" )
        {
            THEN( "it should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( subscription( topic_filter, packet::QoS::AT_LEAST_ONCE ),
                                   io_wally::error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "topic filter \"#\"" )
    {
        auto const topic_filter = "#";
        WHEN( "a caller tries to create a subscription" )
        {
            THEN( "it should NOT see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_NOTHROW( subscription( topic_filter, packet::QoS::AT_LEAST_ONCE ) );
            }
        }
    }

    GIVEN( "topic filter \"+\"" )
    {
        auto const topic_filter = "+";
        WHEN( "a caller tries to create a subscription" )
        {
            THEN( "it should NOT see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_NOTHROW( subscription( topic_filter, packet::QoS::AT_LEAST_ONCE ) );
            }
        }
    }

    GIVEN( "topic filter \"A\"" )
    {
        auto const topic_filter = "A";
        WHEN( "a caller tries to create a subscription" )
        {
            THEN( "it should NOT see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_NOTHROW( subscription( topic_filter, packet::QoS::AT_LEAST_ONCE ) );
            }
        }
    }

    GIVEN( "topic filter \"finance/#\"" )
    {
        auto const topic_filter = "finance/#";
        WHEN( "a caller tries to create a subscription" )
        {
            THEN( "it should NOT see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_NOTHROW( subscription( topic_filter, packet::QoS::AT_LEAST_ONCE ) );
            }
        }
    }

    GIVEN( "topic filter \"/sports/+/finance/+\"" )
    {
        auto const topic_filter = "/sports/+/finance/+";
        WHEN( "a caller tries to create a subscription" )
        {
            THEN( "it should NOT see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_NOTHROW( subscription( topic_filter, packet::QoS::AT_LEAST_ONCE ) );
            }
        }
    }

    GIVEN( "topic filter \"sports/b+\"" )
    {
        auto const topic_filter = "sports/b+";
        WHEN( "a caller tries to create a subscription" )
        {
            THEN( "it should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( subscription( topic_filter, packet::QoS::AT_LEAST_ONCE ),
                                   io_wally::error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "topic filter \"sports/+foo/+\"" )
    {
        auto const topic_filter = "sports/+foo/+";
        WHEN( "a caller tries to create a subscription" )
        {
            THEN( "it should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( subscription( topic_filter, packet::QoS::AT_LEAST_ONCE ),
                                   io_wally::error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "topic filter \"sports/b#\"" )
    {
        auto const topic_filter = "sports/b#";
        WHEN( "a caller tries to create a subscription" )
        {
            THEN( "it should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( subscription( topic_filter, packet::QoS::AT_LEAST_ONCE ),
                                   io_wally::error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "topic filter \"sports/#foo/#\"" )
    {
        auto const topic_filter = "sports/#foo/#";
        WHEN( "a caller tries to create a subscription" )
        {
            THEN( "it should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( subscription( topic_filter, packet::QoS::AT_LEAST_ONCE ),
                                   io_wally::error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "topic filter \"sports/#/\"" )
    {
        auto const topic_filter = "sports/#/";
        WHEN( "a caller tries to create a subscription" )
        {
            THEN( "it should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( subscription( topic_filter, packet::QoS::AT_LEAST_ONCE ),
                                   io_wally::error::malformed_mqtt_packet );
            }
        }
    }
}
