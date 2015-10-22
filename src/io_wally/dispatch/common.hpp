#pragma once

#include <string>
#include <chrono>
#include <memory>

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
        template <typename SENDER>
        struct packet_container
        {
           public:  // static
            using ptr = std::shared_ptr<packet_container<SENDER>>;

            using sender_ptr = std::shared_ptr<SENDER>;

            static packet_container<SENDER>::ptr connect_packet( sender_ptr rx_connection,
                                                                 std::shared_ptr<const protocol::connect> connect )
            {
                return std::make_shared<packet_container<SENDER>>( connect->client_id( ), rx_connection, connect );
            }

            static packet_container<SENDER>::ptr subscribe_packet(
                const std::string& client_id,
                sender_ptr rx_connection,
                std::shared_ptr<const protocol::subscribe> subscribe )
            {
                return std::make_shared<packet_container<SENDER>>( client_id, rx_connection, subscribe );
            }

           public:
            packet_container( const std::string& client_id,
                              sender_ptr rx_connection,
                              std::shared_ptr<const protocol::mqtt_packet> packet )
                : client_id_{client_id}, rx_connection_{rx_connection}, packet_{packet}
            {
                return;
            }

            const std::string& client_id( ) const
            {
                return client_id_;
            }

            std::weak_ptr<SENDER> rx_connection( ) const
            {
                return rx_connection_;
            }

            std::shared_ptr<const protocol::mqtt_packet> packet( )
            {
                return packet_;
            }

           private:
            const long rx_timestamp_{std::chrono::duration_cast<std::chrono::milliseconds>(
                                         std::chrono::system_clock::now( ).time_since_epoch( ) ).count( )};
            const std::string client_id_;
            std::weak_ptr<SENDER> rx_connection_;
            std::shared_ptr<const protocol::mqtt_packet> packet_;
        };  // struct packet_container

    }  // namespace dispatch
}  // namespace io_wally
