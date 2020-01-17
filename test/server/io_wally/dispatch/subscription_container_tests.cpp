#include "catch.hpp"

#include "io_wally/dispatch/topic_subscriptions.hpp"
#include "io_wally/protocol/common.hpp"

using namespace io_wally::protocol;

namespace
{
    const std::string random_topic( size_t length )
    {
        auto randchar = []( ) -> char {
            const char charset[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz/";
            const size_t max_index = ( sizeof( charset ) - 1 );
            return charset[rand( ) % max_index];
        };
        std::string str( length, 0 );
        std::generate_n( str.begin( ), length, randchar );
        return str;
    }
}  // namespace

//
// See: http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718106
//

SCENARIO( "subscription_container#matches", "[dispatch]" )
{
    static const packet::QoS MAX_QOS = packet::QoS::AT_LEAST_ONCE;
    static const std::string CLIENT_ID = "subscription_container_tests";

    GIVEN( "topic filter \"#\" and a random topic" )
    {
        const auto topic_filter = "#";
        const auto topic = random_topic( 1200 );
        const auto under_test = io_wally::dispatch::subscription_container{topic_filter, MAX_QOS, CLIENT_ID};

        WHEN( "a caller matches topic against topic filter " )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( under_test.matches( topic ) == true );
            }
        }
    }

    GIVEN(
        "topic filter \"/sports/premiere-league/barcelona/\" and the same topic "
        "\"/sports/premiere-league/barcelona/\"" )
    {
        const auto topic_filter = "/sports/premiere-league/barcelona/";
        const auto topic = "/sports/premiere-league/barcelona/";
        const auto under_test = io_wally::dispatch::subscription_container{topic_filter, MAX_QOS, CLIENT_ID};

        WHEN( "a caller matches topic against topic filter " )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( under_test.matches( topic ) == true );
            }
        }
    }

    GIVEN( "topic filter \"/sports/premiere-league/barcelona/\" and topic \"sports/premiere-league/barcelona/\"" )
    {
        const auto topic_filter = "/sports/premiere-league/barcelona/";
        const auto topic = "sports/premiere-league/barcelona/";
        const auto under_test = io_wally::dispatch::subscription_container{topic_filter, MAX_QOS, CLIENT_ID};

        WHEN( "a caller matches topic against topic filter " )
        {
            THEN( "it should see the match fail (since topic misses an empty leading topic level)" )
            {
                REQUIRE( under_test.matches( topic ) == false );
            }
        }
    }

    GIVEN( "topic filter \"/sports/premiere-league/barcelona/\" and topic \"/sports/premiere-league/barcelona\"" )
    {
        const auto topic_filter = "/sports/premiere-league/barcelona/";
        const auto topic = "/sports/premiere-league/barcelona";
        const auto under_test = io_wally::dispatch::subscription_container{topic_filter, MAX_QOS, CLIENT_ID};

        WHEN( "a caller matches topic against topic filter" )
        {
            THEN( "it should see the match fail (since topic misses an empty trailing topic level)" )
            {
                REQUIRE( under_test.matches( topic ) == false );
            }
        }
    }

    GIVEN( "topic filter \"/sports/premiere-league/barcelona\" and topic \"/sports/premiere-league/barcelona\"" )
    {
        const auto topic_filter = "/sports/premiere-league/barcelona";
        const auto topic = "/sports/premiere-league/barcelona";
        const auto under_test = io_wally::dispatch::subscription_container{topic_filter, MAX_QOS, CLIENT_ID};

        WHEN( "a caller matches topic against topic filter" )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( under_test.matches( topic ) == true );
            }
        }
    }

    GIVEN( "topic filter \"/sports/+/barcelona\" and topic \"/sports/premiere-league/barcelona\"" )
    {
        const auto topic_filter = "/sports/+/barcelona";
        const auto topic = "/sports/premiere-league/barcelona";
        const auto under_test = io_wally::dispatch::subscription_container{topic_filter, MAX_QOS, CLIENT_ID};

        WHEN( "a caller matches topic against topic filter" )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( under_test.matches( topic ) == true );
            }
        }
    }

    GIVEN( "topic filter \"sport/tennis/player1/#\"" )
    {
        const auto topic_filter = "sport/tennis/player1/#";
        const auto under_test = io_wally::dispatch::subscription_container{topic_filter, MAX_QOS, CLIENT_ID};

        WHEN( "a caller matches topic \"sport/tennis/player1\" against topic filter" )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( under_test.matches( "sport/tennis/player1" ) == true );
            }
        }

        WHEN( "a caller matches topic \"sport/tennis/player1/ranking\" against topic filter" )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( under_test.matches( "sport/tennis/player1/ranking" ) == true );
            }
        }

        WHEN( "a caller matches topic \"sport/tennis/player1/score/wimbledon\" against topic filter" )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( under_test.matches( "sport/tennis/player1/score/wimbledon" ) == true );
            }
        }
    }

    GIVEN( "topic filter \"+/+\" and topic \"/finance\"" )
    {
        const auto topic_filter = "+/+";
        const auto topic = "/finance";
        const auto under_test = io_wally::dispatch::subscription_container{topic_filter, MAX_QOS, CLIENT_ID};

        WHEN( "a caller matches topic against topic filter" )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( under_test.matches( topic ) == true );
            }
        }
    }

    GIVEN( "topic filter \"/+\" and topic \"/finance\"" )
    {
        const auto topic_filter = "/+";
        const auto topic = "/finance";
        const auto under_test = io_wally::dispatch::subscription_container{topic_filter, MAX_QOS, CLIENT_ID};

        WHEN( "a caller matches topic against topic filter" )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( under_test.matches( topic ) == true );
            }
        }
    }

    GIVEN( "topic filter \"+\" and topic \"/finance\"" )
    {
        const auto topic_filter = "+";
        const auto topic = "/finance";
        const auto under_test = io_wally::dispatch::subscription_container{topic_filter, MAX_QOS, CLIENT_ID};

        WHEN( "a caller matches topic against topic filter" )
        {
            THEN( "it should see the match fail" )
            {
                REQUIRE( under_test.matches( topic ) == false );
            }
        }
    }
}

SCENARIO( "subscription_container#topic_filter_matches_one_of", "[dispatch]" )
{
    static const packet::QoS MAX_QOS = packet::QoS::AT_LEAST_ONCE;
    static const std::string CLIENT_ID = "subscription_container_tests";

    GIVEN( "topic filter \"/test\" and topic filters {\"/a\", \"b\", \"/test\"}" )
    {
        const auto topic_filter = "/test";
        const auto topic_filters = std::vector<std::string>{"/a", "/b", "/test"};
        const auto under_test = io_wally::dispatch::subscription_container{topic_filter, MAX_QOS, CLIENT_ID};

        WHEN( "a caller matches topic against topic filters" )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( under_test.topic_filter_matches_one_of( topic_filters ) == true );
            }
        }
    }

    GIVEN( "topic filter \"/another/topic/#\" and topic filters {\"/a\", \"b\", \"/another/topic/+\"}" )
    {
        const auto topic_filter = "/another/topic/#";
        const auto topic_filters = std::vector<std::string>{"/a", "/b", "/another/topic/+"};
        const auto under_test = io_wally::dispatch::subscription_container{topic_filter, MAX_QOS, CLIENT_ID};

        WHEN( "a caller matches topic against topic filters" )
        {
            THEN( "it should see the match fail" )
            {
                REQUIRE( under_test.topic_filter_matches_one_of( topic_filters ) == false );
            }
        }
    }
}
