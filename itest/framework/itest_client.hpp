#pragma once

#include <iostream>
#include <string>
#include <cstdint>
#include <stdexcept>

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

        bool tcp_connect( const std::string& host = "localhost", const std::uint16_t port = 1883 )
        {
            if ( tcp_connected_ )
                throw std::runtime_error( "Client already TCP connected" );

            std::cout << "[itest_client] TCP connecting [host:" << host << "|port:" << std::to_string( port ) << "] ..."
                      << std::endl;
            const int rc = ipstack_.connect( host.c_str( ), port );
            tcp_connected_ = ( rc == 0 ? true : false );
            std::cout << "[itest_client] TCP connected [host:" << host << "|port:" << std::to_string( port )
                      << "]: " << tcp_connected_ << std::endl;

            return tcp_connected_;
        }

        int mqtt_connect( const std::string& client_id,
                          const boost::optional<const std::string>& username = boost::none,
                          const boost::optional<const std::string>& password = boost::none,
                          const unsigned long keep_alive_secs = 0L )
        {
            if ( !tcp_connected_ )
                throw std::runtime_error( "Client not TCP connected" );

            std::cout << "[itest_client] MQTT connecting ..." << std::endl;

            MQTTPacket_connectData connect_packet = MQTTPacket_connectData_initializer;
            connect_packet.MQTTVersion = 3;
            connect_packet.clientID.cstring = const_cast<char*>( client_id.c_str( ) );
            if ( username )
                connect_packet.username.cstring = const_cast<char*>( ( *username ).c_str( ) );
            if ( password )
                connect_packet.password.cstring = const_cast<char*>( ( *password ).c_str( ) );
            if ( keep_alive_secs > 0 )
                connect_packet.keepAliveInterval = keep_alive_secs;

            int rc = client_.connect( connect_packet );
            std::cout << "[itest_client] MQTT connected: rc = " << std::to_string( rc ) << std::endl;

            return rc;
        }

        int mqtt_disconnect( )
        {
            return client_.disconnect( );
        }

        bool is_tcp_connected( )
        {
            // return tcp_connected_;
            return ipstack_.is_connected( );
        }

        bool is_mqtt_connected( )
        {
            return client_.isConnected( );
        }

        bool tcp_disconnect( )
        {
            const int rc = ipstack_.disconnect( );

            return ( rc == 0 ? true : false );
        }

       private:
        bool tcp_connected_{false};
        IPStack ipstack_{};
        MQTT::Client<IPStack, Countdown> client_{ipstack_};
    };

}  // namespace framework
