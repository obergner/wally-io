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

        bool connect( const std::string& host = "localhost", const std::uint16_t port = 1883 )
        {
            const int rc = ipstack_.connect( host.c_str( ), port );

            return ( rc == 0 ? true : false );
        }

        int send_connect_packet( const std::string& client_id,
                                 const boost::optional<const std::string>& username = boost::none,
                                 const boost::optional<const std::string>& password = boost::none,
                                 const unsigned long keep_alive_secs = 0L )
        {
            MQTTPacket_connectData connect_packet = MQTTPacket_connectData_initializer;
            connect_packet.MQTTVersion = 3;
            connect_packet.clientID.cstring = const_cast<char*>( client_id.c_str( ) );
            if ( username )
                connect_packet.username.cstring = const_cast<char*>( ( *username ).c_str( ) );
            if ( password )
                connect_packet.password.cstring = const_cast<char*>( ( *password ).c_str( ) );
            if ( keep_alive_secs > 0 )
                connect_packet.keepAliveInterval = keep_alive_secs;

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
