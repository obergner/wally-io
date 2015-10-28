#pragma once

#include <memory>
#include <iostream>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <mutex>
#include <condition_variable>

#include <boost/optional.hpp>

#include "MQTTClient.h"

namespace framework
{
    class itest_client final
    {
       public:
        itest_client( const std::string& client_id = "itest_client" )
        {
            auto const uri = "tcp://127.0.0.1:1883";
            MQTTClient_create( &client_, uri, client_id.c_str( ), MQTTCLIENT_PERSISTENCE_NONE, nullptr );
        }

        virtual ~itest_client( )
        {
            MQTTClient_destroy( &client_ );
        }

        int mqtt_connect( const boost::optional<const std::string>& username = boost::none,
                          const boost::optional<const std::string>& password = boost::none,
                          const unsigned long keep_alive_secs = 0L )
        {
            std::cout << "[itest_client] MQTT connecting ..." << std::endl;

            MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
            if ( username )
                conn_opts.username = ( *username ).c_str( );
            if ( password )
                conn_opts.password = ( *password ).c_str( );
            if ( keep_alive_secs > 0 )
                conn_opts.keepAliveInterval = keep_alive_secs;

            auto const rc = MQTTClient_connect( client_, &conn_opts );

            std::cout << "[itest_client] MQTT connected: rc = " << std::to_string( rc ) << std::endl;

            return rc;
        }

        int mqtt_disconnect( )
        {
            return MQTTClient_disconnect( client_, 0 );
        }

        bool is_mqtt_connected( )
        {
            return ( MQTTClient_isConnected( client_ ) != 0 );
        }

       private:
        MQTTClient client_{};
    };
}  // namespace framework
