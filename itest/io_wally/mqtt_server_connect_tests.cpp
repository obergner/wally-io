#include "catch.hpp"

#include <boost/optional.hpp>

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
            auto connected = client.connect( );

            THEN( "it will succeed" )
            {
                REQUIRE( connected );
            }
        }
    }
}

SCENARIO_METHOD( itest_fixture, "mqtt_server MQTT CONNECT tests", "[mqtt_server]" )
{
    GIVEN( "a running mqtt_server instance" )
    {
        WHEN( "a client sends a valid CONNECT packet to that server" )
        {
            auto connected = client.connect( );
            auto login_rc = client.send_connect_packet( "CONNECT_itest" );

            THEN( "it will succeed" )
            {
                REQUIRE( connected );
                REQUIRE( login_rc == 0 );
            }
        }

        // WHEN( "a client sends a CONNECT packet with keep alive interval set to that server" )
        // {
        //     auto connected = client.connect( );
        //     auto login_rc = client.send_connect_packet( "CONNECT_itest", boost::none, boost::none, 1L );

        //     THEN( "it will successfully connect" )
        //     {
        //         REQUIRE( connected );
        //         REQUIRE( login_rc == 0 );
        //     }

        //     AND_THEN( "it will be disconnected after keep alive interval expires" )
        //     {
        //         sleep( 2 );
        //         REQUIRE( !client.is_connected( ) );
        //     }
        // }

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
