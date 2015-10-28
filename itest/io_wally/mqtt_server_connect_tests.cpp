#include "catch.hpp"

#include <boost/optional.hpp>

#include "framework/itest_client.hpp"

namespace
{
    class itest_fixture
    {
       protected:
        framework::itest_client client{};
    };  // class itest_fixture
}  // namespace

SCENARIO_METHOD( itest_fixture, "mqtt_server MQTT CONNECT tests", "[mqtt_server]" )
{
    GIVEN( "a running mqtt_server instance" )
    {
        WHEN( "a client sends a valid CONNECT packet to that server" )
        {
            auto login_rc = client.mqtt_connect( );

            THEN( "it will succeed" )
            {
                REQUIRE( login_rc == 0 );
            }
        }

        WHEN( "a client sends a CONNECT packet with keep alive interval set to that server" )
        {
            auto login_rc = client.mqtt_connect( boost::none, boost::none, 1L );

            THEN( "it will successfully connect" )
            {
                REQUIRE( login_rc == 0 );
            }

            AND_THEN( "it will be disconnected after keep alive interval expires" )
            {
                sleep( 3 );
                REQUIRE( !client.is_mqtt_connected( ) );
            }
        }

        //        WHEN( "a client sends a second CONNECT packet on the same connection to that server" )
        //        {
        //            const bool connected_before = client.connect( );
        //            const int login_before_rc = client.send_connect_packet( "CONNECT_itest" );
        //
        //            const int login_after_rc = client.send_connect_packet( "CONNECT_itest" );
        //
        //            THEN( "it will fail" )
        //            {
        //                REQUIRE( connected_before );
        //                REQUIRE( login_before_rc == 0 );
        //
        //                REQUIRE( login_after_rc != 0 );
        //            }
        //        }
    }
}
