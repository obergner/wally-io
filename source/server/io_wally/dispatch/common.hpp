#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/connect_packet.hpp"
#include "io_wally/protocol/disconnect_packet.hpp"
#include "io_wally/protocol/puback_packet.hpp"
#include "io_wally/protocol/pubcomp_packet.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/pubrec_packet.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"

namespace io_wally::dispatch
{
    /// Represents a resolved topic subscriber: when processing a PUBLISH packet an MQTT broker may conclude
    /// (correctly) that a client has registered more than one topic filter matching that PUBLISH packet's
    /// topic. In this case, MQTT 3.1.1 mandates that the PUBLISH packet be delivered to that client only
    /// once, using the maximum QoS of all matching subscriptions.
    ///
    /// This type represents this resolution process' result.
    using resolved_subscriber_t = std::pair<const std::string, protocol::packet::QoS>;

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
        /// Connection was disconnected due to client authentication failure.
        authentication_failed,
        /// Connection was disconnected due to a protocol violation, e.g. a client sent more than one CONNECT
        /// packet.
        protocol_violation,
        /// Connection was disconnected after keep alive timeout expired
        keep_alive_timeout_expired,
        /// Connection was lost/disconnected due to a network error or server failure.
        network_or_server_failure
    };

    /// \brief Overload stream output operator for \c packet::QoS.
    ///
    /// Overload stream output operator for \c packet::QoS, primarily meant to facilitate logging.
    inline auto operator<<( std::ostream& output, disconnect_reason const& rsn ) -> std::ostream&
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
            case disconnect_reason::authentication_failed:
                repr = "Authentication failed";
                break;
            case disconnect_reason::protocol_violation:
                repr = "Protocol violation";
                break;
            case disconnect_reason::keep_alive_timeout_expired:
                repr = "Keep alive timeout expired";
                break;
            case disconnect_reason::network_or_server_failure:
                repr = "Network or server failure";
                break;
            default:
                assert( false );
                break;
        }
        output << repr;

        return output;
    }

    template <typename SENDER>
    struct packet_container final
    {
       public:  // static
        using ptr = std::shared_ptr<packet_container<SENDER>>;

        using sender_ptr = std::shared_ptr<SENDER>;

        static auto contain( const std::string& client_id,
                             const std::shared_ptr<SENDER>& rx_connection,
                             std::shared_ptr<protocol::mqtt_packet> packet,
                             const dispatch::disconnect_reason disconnect_reason =
                                 dispatch::disconnect_reason::not_a_disconnect ) -> packet_container<SENDER>::ptr
        {
            return std::make_shared<packet_container<SENDER>>( client_id, rx_connection, packet, disconnect_reason );
        }

       public:
        packet_container( std::string client_id,
                          const std::shared_ptr<SENDER>& rx_connection,
                          std::shared_ptr<protocol::mqtt_packet> packet,
                          const dispatch::disconnect_reason disconnect_reason )
            : client_id_{std::move( client_id )},
              rx_connection_{rx_connection},
              packet_{std::move( packet )},
              disconnect_reason_{disconnect_reason}
        {
            return;
        }

        [[nodiscard]] auto client_id( ) const -> const std::string&
        {
            return client_id_;
        }

        auto rx_connection( ) const -> std::weak_ptr<SENDER>
        {
            return rx_connection_;
        }

        [[nodiscard]] auto packet_type( ) const -> protocol::packet::Type
        {
            return packet_->type( );
        }

        auto packet( ) -> std::shared_ptr<protocol::mqtt_packet>
        {
            return packet_;
        }

        [[nodiscard]] auto disconnect_reason( ) const -> dispatch::disconnect_reason
        {
            return disconnect_reason_;
        }

        template <typename PACKET>
        auto packet_as( ) -> std::shared_ptr<PACKET>
        {
            static_assert( std::is_base_of<protocol::mqtt_packet, PACKET>::value,
                           "Template parameter PACKET needs to be a subtype of protocol::mqtt_packet" );
            return std::dynamic_pointer_cast<PACKET>( packet_ );
        }

       private:
        const std::string client_id_;
        std::weak_ptr<SENDER> rx_connection_;
        std::shared_ptr<protocol::mqtt_packet> packet_;
        const dispatch::disconnect_reason disconnect_reason_;
    };  // struct packet_container

    auto topic_filter_matches_topic( const std::string& topic_filter, const std::string& topic ) -> bool;
}  // namespace io_wally::dispatch
