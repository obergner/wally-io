#pragma once

#include <string>
#include <chrono>

#include "io_wally/mqtt_connection.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/connect_packet.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"

namespace io_wally
{
    /// \brief Namespace grouping all classes, structs etc. dealing with \c dispatching \c mqtt_packets.
    ///
    /// There are predominantly three types of \c mqtt_packets logic contained in this namespace deals with:
    ///
    ///  - \c protocol::connect: Any \c connect packet - provided it results in successful client authentication - will
    ///                          be dispatched to \c mqtt_client_session_manager to establish a new
    ///                          \c mqtt_client_session.
    ///  - \c protocol::subscribe Any received \c subscribe packet will be forwarded to XXX to register its
    ///                           \c subscriptions.
    ///  - \c protocol::publish Any received \c publish packet will be forwarded to XXX for publishing to interested
    ///                         clients.
    namespace dispatch
    {
        struct packet_container
        {
           public:  // static
            static packet_container connect_packet( const std::string& client_id,
                                                    mqtt_connection::ptr rx_connection,
                                                    std::shared_ptr<protocol::connect> connect )
            {
                return packet_container{client_id, rx_connection, connect};
            }

            static packet_container subscribe_packet( const std::string& client_id,
                                                      mqtt_connection::ptr rx_connection,
                                                      std::shared_ptr<protocol::subscribe> subscribe )
            {
                return packet_container{client_id, rx_connection, subscribe};
            }

           public:
            const std::string& client_id( ) const
            {
                return client_id_;
            }

            std::weak_ptr<mqtt_connection> rx_connection( ) const
            {
                return rx_connection_;
            }

            std::shared_ptr<protocol::mqtt_packet> packet( )
            {
                return packet_;
            }

           private:
            packet_container( const std::string& client_id,
                              mqtt_connection::ptr rx_connection,
                              std::shared_ptr<protocol::mqtt_packet> packet )
                : client_id_{client_id}, rx_connection_{rx_connection}, packet_{packet}
            {
                return;
            }

           private:
            const long rx_timestamp_{std::chrono::duration_cast<std::chrono::milliseconds>(
                                         std::chrono::system_clock::now( ).time_since_epoch( ) ).count( )};
            const std::string client_id_;
            std::weak_ptr<mqtt_connection> rx_connection_;
            std::shared_ptr<protocol::mqtt_packet> packet_;
        };  // struct packet_container

    }  // namespace dispatch
}  // namespace io_wally
