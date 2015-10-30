#include "catch.hpp"

#include <boost/optional.hpp>

#include "framework/itest_client.hpp"

#include "io_wally/protocol/common.hpp"

namespace
{
    class itest_fixture
    {
       protected:
        framework::itest_client subscriber{"itest_client:subscriber"};
        framework::itest_client publisher{"itest_client:publisher"};
    };  // class itest_fixture
}  // namespace

SCENARIO_METHOD( itest_fixture, "mqtt_server MQTT PUBLISH tests", "[mqtt_server]" )
{
    GIVEN( "a client subscribed to a topic" )
    {
        auto login_rc = subscriber.mqtt_connect( );
        auto subscribe_rc = subscriber.subscribe( "test/topic", io_wally::protocol::packet::QoS::AT_MOST_ONCE );

        WHEN( "another client sends a valid PUBLISH packet to that topic" )
        {
            auto login_pub_rc = publisher.mqtt_connect( );
            auto send_pub_rc = publisher.publish( "test/topic", "test" );

            THEN( "it will succeed" )
            {
                REQUIRE( login_rc == 0 );
                REQUIRE( subscribe_rc == 0 );
                REQUIRE( login_pub_rc == 0 );
                REQUIRE( send_pub_rc == 0 );
            }
        }
    }
}
