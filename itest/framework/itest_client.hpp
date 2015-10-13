#pragma once

#include <string>
#include <cstdint>

#include <boost/optional.hpp>

#define MQTTCLIENT_QOS2 1

#include "MQTTClient.h"

#define DEFAULT_STACK_SIZE -1

#include "linux.cpp"

namespace framework
{
    using namespace std;

    class itest_client
    {
       public:
        itest_client( )
        {
        }

        virtual ~itest_client( )
        {
            ipstack_.disconnect( );
        }

        bool connect( const string& host = "localhost", const uint16_t port = 1883 )
        {
            const int rc = ipstack_.connect( host.c_str( ), port );

            return ( rc == 0 ? true : false );
        }

        int send_connect_packet( const string& client_id,
                                 const boost::optional<const string>& username = boost::none,
                                 const boost::optional<const string>& password = boost::none )
        {
            MQTTPacket_connectData connect_packet = MQTTPacket_connectData_initializer;
            connect_packet.MQTTVersion = 3;
            connect_packet.clientID.cstring = const_cast<char*>( client_id.c_str( ) );
            if ( username )
                connect_packet.username.cstring = const_cast<char*>( ( *username ).c_str( ) );
            if ( password )
                connect_packet.password.cstring = const_cast<char*>( ( *password ).c_str( ) );

            return client_.connect( connect_packet );
        }

        int send_disconnect_packet( )
        {
            return client_.disconnect( );
        }

        bool is_connected( )
        {
            return client_.isConnected( );
        }

        bool disconnect( )
        {
            const int rc = ipstack_.disconnect( );

            return ( rc == 0 ? true : false );
        }

       private:
        IPStack ipstack_{};

        MQTT::Client<IPStack, Countdown> client_{ipstack_};
    };

}  // namespace framework
