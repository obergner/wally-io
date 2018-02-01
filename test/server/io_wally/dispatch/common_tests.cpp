#include "catch.hpp"

#include "io_wally/dispatch/common.hpp"

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
}

//
// See: http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718106
//

SCENARIO( "topic_filter_matches_topic", "[dispatch]" )
{
    GIVEN( "topic filter \"#\" and a random topic" )
    {
        const auto topic_filter = "#";
        const auto topic = random_topic( 1200 );

        WHEN( "a caller matches topic against topic filter " )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( io_wally::dispatch::topic_filter_matches_topic( topic_filter, topic ) == true );
            }
        }
    }

    GIVEN(
        "topic filter \"/sports/premiere-league/barcelona/\" and the same topic "
        "\"/sports/premiere-league/barcelona/\"" )
    {
        const auto topic_filter = "/sports/premiere-league/barcelona/";
        const auto topic = "/sports/premiere-league/barcelona/";

        WHEN( "a caller matches topic against topic filter " )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( io_wally::dispatch::topic_filter_matches_topic( topic_filter, topic ) == true );
            }
        }
    }

    GIVEN( "topic filter \"/sports/premiere-league/barcelona/\" and topic \"sports/premiere-league/barcelona/\"" )
    {
        const auto topic_filter = "/sports/premiere-league/barcelona/";
        const auto topic = "sports/premiere-league/barcelona/";

        WHEN( "a caller matches topic against topic filter " )
        {
            THEN( "it should see the match fail (since topic misses an empty leading topic level)" )
            {
                REQUIRE( io_wally::dispatch::topic_filter_matches_topic( topic_filter, topic ) == false );
            }
        }
    }

    GIVEN( "topic filter \"/sports/premiere-league/barcelona/\" and topic \"/sports/premiere-league/barcelona\"" )
    {
        const auto topic_filter = "/sports/premiere-league/barcelona/";
        const auto topic = "/sports/premiere-league/barcelona";

        WHEN( "a caller matches topic against topic filter" )
        {
            THEN( "it should see the match fail (since topic misses an empty trailing topic level)" )
            {
                REQUIRE( io_wally::dispatch::topic_filter_matches_topic( topic_filter, topic ) == false );
            }
        }
    }

    GIVEN( "topic filter \"/sports/premiere-league/barcelona\" and topic \"/sports/premiere-league/barcelona\"" )
    {
        const auto topic_filter = "/sports/premiere-league/barcelona";
        const auto topic = "/sports/premiere-league/barcelona";

        WHEN( "a caller matches topic against topic filter" )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( io_wally::dispatch::topic_filter_matches_topic( topic_filter, topic ) == true );
            }
        }
    }

    GIVEN( "topic filter \"/sports/+/barcelona\" and topic \"/sports/premiere-league/barcelona\"" )
    {
        const auto topic_filter = "/sports/+/barcelona";
        const auto topic = "/sports/premiere-league/barcelona";

        WHEN( "a caller matches topic against topic filter" )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( io_wally::dispatch::topic_filter_matches_topic( topic_filter, topic ) == true );
            }
        }
    }

    GIVEN( "topic filter \"sport/tennis/player1/#\"" )
    {
        const auto topic_filter = "sport/tennis/player1/#";

        WHEN( "a caller matches topic \"sport/tennis/player1\" against topic filter" )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( io_wally::dispatch::topic_filter_matches_topic( topic_filter, "sport/tennis/player1" ) ==
                         true );
            }
        }

        WHEN( "a caller matches topic \"sport/tennis/player1/ranking\" against topic filter" )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( io_wally::dispatch::topic_filter_matches_topic( topic_filter,
                                                                         "sport/tennis/player1/ranking" ) == true );
            }
        }

        WHEN( "a caller matches topic \"sport/tennis/player1/score/wimbledon\" against topic filter" )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( io_wally::dispatch::topic_filter_matches_topic(
                             topic_filter, "sport/tennis/player1/score/wimbledon" ) == true );
            }
        }
    }

    GIVEN( "topic filter \"+/+\" and topic \"/finance\"" )
    {
        const auto topic_filter = "+/+";
        const auto topic = "/finance";

        WHEN( "a caller matches topic against topic filter" )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( io_wally::dispatch::topic_filter_matches_topic( topic_filter, topic ) == true );
            }
        }
    }

    GIVEN( "topic filter \"/+\" and topic \"/finance\"" )
    {
        const auto topic_filter = "/+";
        const auto topic = "/finance";

        WHEN( "a caller matches topic against topic filter" )
        {
            THEN( "it should see the match succeed" )
            {
                REQUIRE( io_wally::dispatch::topic_filter_matches_topic( topic_filter, topic ) == true );
            }
        }
    }

    GIVEN( "topic filter \"+\" and topic \"/finance\"" )
    {
        const auto topic_filter = "+";
        const auto topic = "/finance";

        WHEN( "a caller matches topic against topic filter" )
        {
            THEN( "it should see the match fail" )
            {
                REQUIRE( io_wally::dispatch::topic_filter_matches_topic( topic_filter, topic ) == false );
            }
        }
    }
}
