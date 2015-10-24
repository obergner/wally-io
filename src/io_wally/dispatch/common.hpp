#pragma once

#include <cstdint>
#include <string>
#include <chrono>
#include <memory>

#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/connect_packet.hpp"
#include "io_wally/protocol/disconnect_packet.hpp"
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
    ///  - \c protocol::disconnect: Any \c disconnect packet will be dispatched to \c mqtt_client_session_manager to
    ///                             destroy the \c mqtt_client_session associated with the client just disconnected (if
    ///                             any)
    ///  - \c protocol::subscribe Any received \c subscribe packet will be forwarded to XXX to register its
    ///                           \c subscriptions.
    ///  - \c protocol::publish Any received \c publish packet will be forwarded to XXX for publishing to interested
    ///                         clients.
    namespace dispatch
    {
        /// \brief Flags reason for why a client disconnected.
        ///
        /// We do not only forward DISCONNECT requests received from clients, but also generate "fake" DISCONNECT
        /// packets if a client connection was closed for other reasons, i.e. because of protocol violation or
        /// network errors. This enum allows dispatcher subsystem to differentiate between those different types
        /// of disconnect packets.
        enum class disconnect_reason : uint8_t
        {
            /// Packet is not a DISCONNECT packet.
            not_a_disconnect = 0,
            /// Regular disconnect: DISCONNECT request received from client.
            client_disconnect,
            /// Connection was disconnected due to a protocol violation, e.g. a client sent more than one CONNECT
            /// packet.
            protocol_violation,
            /// Connection was lost/disconnected due to a network error or server failure.
            connection_lost
        };

        /// \brief Overload stream output operator for \c packet::QoS.
        ///
        /// Overload stream output operator for \c packet::QoS, primarily meant to facilitate logging.
        inline std::ostream& operator<<( std::ostream& output, disconnect_reason const& rsn )
        {
            std::string repr;
            switch ( rsn )
            {
                case disconnect_reason::not_a_disconnect:
                    repr = "Not a DISCONNECT";
                    break;
                case disconnect_reason::client_disconnect:
                    repr = "Client disconnect";
                    break;
                case disconnect_reason::protocol_violation:
                    repr = "Protocol violation";
                    break;
                case disconnect_reason::connection_lost:
                    repr = "Connection lost";
                    break;
                default:
                    assert( false );
                    break;
            }
            output << repr;

            return output;
        }
        template <typename SENDER>
        struct packet_container
        {
           public:  // nested types
           public:  // static
            using ptr = std::shared_ptr<packet_container<SENDER>>;

            using sender_ptr = std::shared_ptr<SENDER>;

            static packet_container<SENDER>::ptr connect_packet( sender_ptr rx_connection,
                                                                 std::shared_ptr<const protocol::connect> connect )
            {
                return std::make_shared<packet_container<SENDER>>(
                    connect->client_id( ), rx_connection, connect, disconnect_reason::not_a_disconnect );
            }

            static packet_container<SENDER>::ptr disconnect_packet(
                const std::string& client_id,
                sender_ptr rx_connection,
                std::shared_ptr<const protocol::disconnect> disconnect,
                const disconnect_reason disconnect_reason = disconnect_reason::client_disconnect )
            {
                return std::make_shared<packet_container<SENDER>>(
                    client_id, rx_connection, disconnect, disconnect_reason );
            }

            static packet_container<SENDER>::ptr subscribe_packet(
                const std::string& client_id,
                sender_ptr rx_connection,
                std::shared_ptr<const protocol::subscribe> subscribe )
            {
                return std::make_shared<packet_container<SENDER>>(
                    client_id, rx_connection, subscribe, disconnect_reason::not_a_disconnect );
            }

           public:
            packet_container( const std::string& client_id,
                              sender_ptr rx_connection,
                              std::shared_ptr<const protocol::mqtt_packet> packet,
                              const disconnect_reason disconnect_reason )
                : client_id_{client_id},
                  rx_connection_{rx_connection},
                  packet_{packet},
                  disconnect_reason_{disconnect_reason}
            {
                return;
            }

            const std::chrono::system_clock::time_point received( ) const
            {
                return std::chrono::system_clock::from_time_t( rx_timestamp_ );
            }

            const std::string& client_id( ) const
            {
                return client_id_;
            }

            std::weak_ptr<SENDER> rx_connection( ) const
            {
                return rx_connection_;
            }

            protocol::packet::Type packet_type( ) const
            {
                return packet_->header( ).type( );
            }

            std::shared_ptr<const protocol::mqtt_packet> packet( )
            {
                return packet_;
            }

            template <typename PACKET>
            std::shared_ptr<const PACKET> packetAs( )
            {
                static_assert( std::is_base_of<protocol::mqtt_packet, PACKET>::value,
                               "Template parameter PACKET needs to be a subtype of protocol::mqtt_packet" );
                return std::dynamic_pointer_cast<const PACKET>( packet_ );
            }

           private:
            const std::time_t rx_timestamp_{std::chrono::duration_cast<std::chrono::milliseconds>(
                                                std::chrono::system_clock::now( ).time_since_epoch( ) ).count( )};
            const std::string client_id_;
            std::weak_ptr<SENDER> rx_connection_;
            std::shared_ptr<const protocol::mqtt_packet> packet_;
            const disconnect_reason disconnect_reason_;
        };  // struct packet_container

    }  // namespace dispatch
}  // namespace io_wally
