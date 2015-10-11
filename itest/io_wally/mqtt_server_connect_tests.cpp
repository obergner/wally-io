#include "catch.hpp"

#include "framework/itest_client.hpp"
#include "framework/server_controller.hpp"

namespace
{
    class itest_fixture
    {
       protected:
        framework::server_controller server_{};

        framework::itest_client client{};
    };  // class itest_fixture
}  // namespace

SCENARIO_METHOD( itest_fixture, "mqtt_server TCP connection tests", "[mqtt_server]" )
{
    GIVEN( "a running mqtt_server instance" )
    {
        WHEN( "a client tries to open a TCP connection to that server" )
        {
            const int connect_rc = client.connect( );

            THEN( "it will succeed" )
            {
                REQUIRE( connect_rc == 0 );
            }
        }
    }
}

// SCENARIO_METHOD( itest_fixture, "mqtt_server MQTT CONNECT tests", "[mqtt_server]" )
//{
//    GIVEN( "a running mqtt_server instance" )
//    {
//        WHEN( "a client sends a valid CONNECT packet to that server" )
//        {
//            const int connect_rc = client.connect( );
//            const int login_rc = client.send_connect_packet( "CONNECT_itest" );
//
//            THEN( "it will succeed" )
//            {
//                REQUIRE( connect_rc == 0 );
//                REQUIRE( login_rc == 0 );
//            }
//        }
//    }
//}
