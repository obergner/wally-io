#pragma once

#include <stdio.h>

#include <iostream>
#include <string>
#include <cstdint>

#include <boost/optional.hpp>

#include "io_wally/protocol/common.hpp"

#include "MQTTClient.h"

namespace framework
{
    class itest_client final
    {
       public:
        itest_client( const std::string& client_id = "itest_client" ) : client_id_{client_id}
        {
            auto const uri = "tcp://127.0.0.1:1883";
            MQTTClient_create( &client_, uri, client_id_.c_str( ), MQTTCLIENT_PERSISTENCE_NONE, nullptr );
        }

        virtual ~itest_client( )
        {
            MQTTClient_destroy( &client_ );
        }

        int mqtt_connect( const boost::optional<const std::string>& username = boost::none,
                          const boost::optional<const std::string>& password = boost::none,
                          const unsigned long keep_alive_secs = 0L )
        {
            log( ) << "MQTT connecting ..." << std::endl;

            MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
            if ( username )
                conn_opts.username = ( *username ).c_str( );
            if ( password )
                conn_opts.password = ( *password ).c_str( );
            if ( keep_alive_secs > 0 )
                conn_opts.keepAliveInterval = keep_alive_secs;

            auto const rc = MQTTClient_connect( client_, &conn_opts );

            log( ) << "MQTT connected: rc = " << std::to_string( rc ) << std::endl;

            return rc;
        }

        int subscribe( const std::string& topic, const io_wally::protocol::packet::QoS maximum_qos )
        {
            log( ) << "Subscribing to topic = \"" << topic << "\" with maximum QoS = " << maximum_qos << " ..."
                   << std::endl;
            auto rc = MQTTClient_subscribe( client_, topic.c_str( ), static_cast<int>( maximum_qos ) );
            log( ) << "Subscribed to topic = \"" << topic << "\" with maximum QoS = " << maximum_qos << ": rc = " << rc
                   << std::endl;
            return rc;
        }

        const boost::optional<const std::string> receive_message( const uint16_t timeout_ms )
        {
            auto topic_name = ( char* ){nullptr};
            auto topic_len = int{};
            auto message = ( MQTTClient_message* ){nullptr};

            log( ) << "Receiving message ..." << std::endl;
            auto const rc = MQTTClient_receive( client_, &topic_name, &topic_len, &message, timeout_ms );
            log( ) << "Received message: [rc:" << rc << "|msglen:" << message->payloadlen << "]" << std::endl;

            auto msg = boost::optional<const std::string>{};
            if ( message != nullptr )
            {
                auto len = static_cast<size_t>( message->payloadlen );
                msg.emplace( std::string{static_cast<char*>( message->payload ), len} );
            }
            else
            {
                msg = boost::none;
            }
            MQTTClient_freeMessage( &message );
            MQTTClient_free( &topic_name );

            return msg;
        }

        int publish( const std::string& topic,
                     const std::string& message,
                     const io_wally::protocol::packet::QoS qos = io_wally::protocol::packet::QoS::AT_MOST_ONCE,
                     const bool retain = false )
        {
            log( ) << "Publishing: [topic:" << topic << "|msg:" << message << "|qos:" << qos << "ret:" << retain
                   << "] ..." << std::endl;
            auto rc = MQTTClient_publish( client_,
                                          topic.c_str( ),
                                          message.length( ),
                                          const_cast<void*>( static_cast<const void*>( message.c_str( ) ) ),
                                          static_cast<int>( qos ),
                                          ( retain ? 1 : 0 ),
                                          nullptr );
            log( ) << "Published: [topic:" << topic << "|msg:" << message << "|qos:" << qos << "ret:" << retain
                   << "]: rc = " << rc << std::endl;

            return rc;
        }

        int mqtt_disconnect( )
        {
            return MQTTClient_disconnect( client_, 0 );
        }

        bool is_mqtt_connected( )
        {
            return ( MQTTClient_isConnected( client_ ) == 0 );
        }

       private:
        std::ostream& log( )
        {
            std::cout << "[" << client_id_ << "] ";
            return std::cout;
        }

       private:
        const std::string client_id_;
        MQTTClient client_{};
    };
}  // namespace framework
