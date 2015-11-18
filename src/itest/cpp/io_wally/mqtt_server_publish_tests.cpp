#include "catch.hpp"

#include <boost/optional.hpp>

#include "framework/itest_client.hpp"

#include "io_wally/protocol/common.hpp"

namespace
{
    class itest_fixture_qos0
    {
       protected:
        framework::itest_client subscriber{"publish_tests:qos0:subscriber"};
        framework::itest_client publisher{"publish_tests:qos0:publisher"};
    };  // class itest_fixture

    class itest_fixture_qos1
    {
       protected:
        framework::itest_client subscriber{"publish_tests:qos1:subscriber"};
        framework::itest_client publisher{"publish_tests:qos1:publisher"};
    };  // class itest_fixture

    class itest_fixture_qos2
    {
       protected:
        framework::itest_client subscriber{"publish_tests:qos2:subscriber"};
        framework::itest_client publisher{"publish_tests:qos2:publisher"};
    };  // class itest_fixture
}  // namespace

SCENARIO_METHOD( itest_fixture_qos0, "mqtt_server MQTT PUBLISH tests (QoS 0)", "[mqtt_server]" )
{
    GIVEN( "a client subscribed to a topic with QoS 0" )
    {
        auto login_rc = subscriber.mqtt_connect( );
        auto subscribe_rc = subscriber.subscribe( "test/topic/qos0", io_wally::protocol::packet::QoS::AT_MOST_ONCE );

        WHEN( "another client sends a valid PUBLISH packet with QoS 0 to that topic" )
        {
            auto login_pub_rc = publisher.mqtt_connect( );
            auto send_pub_rc =
                publisher.publish( "test/topic/qos0", "test/qos0", io_wally::protocol::packet::QoS::AT_MOST_ONCE );

            auto recvd_msg = subscriber.receive_message( 1000 );

            THEN( "it will succeed" )
            {
                REQUIRE( login_rc == 0 );
                REQUIRE( subscribe_rc == 0 );
                REQUIRE( login_pub_rc == 0 );
                REQUIRE( send_pub_rc == 0 );
                REQUIRE( recvd_msg );
                REQUIRE( *recvd_msg == "test/qos0" );
            }
        }
    }
}

SCENARIO_METHOD( itest_fixture_qos1, "mqtt_server MQTT PUBLISH tests (QoS 1)", "[mqtt_server]" )
{
    GIVEN( "a client subscribed to a topic with QoS 1" )
    {
        auto login_rc = subscriber.mqtt_connect( );
        auto subscribe_rc = subscriber.subscribe( "test/topic/qos1", io_wally::protocol::packet::QoS::AT_LEAST_ONCE );

        WHEN( "another client sends a valid PUBLISH packet with QoS 1 to that topic" )
        {
            auto login_pub_rc = publisher.mqtt_connect( );
            auto send_pub_rc =
                publisher.publish( "test/topic/qos1", "test/qos1", io_wally::protocol::packet::QoS::AT_LEAST_ONCE );

            auto recvd_msg = subscriber.receive_message( 1000 );

            THEN( "it will succeed" )
            {
                REQUIRE( login_rc == 0 );
                REQUIRE( subscribe_rc == 0 );
                REQUIRE( login_pub_rc == 0 );
                REQUIRE( send_pub_rc == 0 );
                REQUIRE( recvd_msg );
                REQUIRE( *recvd_msg == "test/qos1" );
            }
        }
    }
}

SCENARIO_METHOD( itest_fixture_qos2, "mqtt_server MQTT PUBLISH tests (QoS 2)", "[mqtt_server]" )
{
    GIVEN( "a client subscribed to a topic with QoS 2" )
    {
        auto login_rc = subscriber.mqtt_connect( );
        auto subscribe_rc = subscriber.subscribe( "test/topic/qos2", io_wally::protocol::packet::QoS::EXACTLY_ONCE );

        WHEN( "another client sends a valid PUBLISH packet with QoS 2 to that topic" )
        {
            auto login_pub_rc = publisher.mqtt_connect( );
            auto send_pub_rc =
                publisher.publish( "test/topic/qos2", "test/qos2", io_wally::protocol::packet::QoS::EXACTLY_ONCE );

            auto recvd_msg = subscriber.receive_message( 1000 );

            THEN( "it will succeed" )
            {
                REQUIRE( login_rc == 0 );
                REQUIRE( subscribe_rc == 0 );
                REQUIRE( login_pub_rc == 0 );
                REQUIRE( send_pub_rc == 0 );
                REQUIRE( recvd_msg );
                REQUIRE( *recvd_msg == "test/qos2" );
            }
        }
    }
}
