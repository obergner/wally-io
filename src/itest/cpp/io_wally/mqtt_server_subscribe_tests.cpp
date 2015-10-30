#include "catch.hpp"

#include <boost/optional.hpp>

#include "framework/itest_client.hpp"

#include "io_wally/protocol/common.hpp"

namespace
{
    class itest_fixture
    {
       protected:
        framework::itest_client client{};
    };  // class itest_fixture
}  // namespace

SCENARIO_METHOD( itest_fixture, "mqtt_server MQTT SUBSCRIBE tests", "[mqtt_server]" )
{
    GIVEN( "a running mqtt_server instance" )
    {
        WHEN( "a client sends a valid SUBSCRIBE packet to that server" )
        {
            auto login_rc = client.mqtt_connect( );
            auto subscribe_rc = client.subscribe( "test/topic", io_wally::protocol::packet::QoS::AT_MOST_ONCE );

            THEN( "it will succeed" )
            {
                REQUIRE( login_rc == 0 );
                REQUIRE( subscribe_rc == 0 );
            }
        }
    }
}
