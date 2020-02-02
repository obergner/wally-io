#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>

#include <asio.hpp>

#include "io_wally/context.hpp"
#include "io_wally/dispatch/tx_publication.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_ack_packet.hpp"
#include "io_wally/protocol/publish_packet.hpp"

namespace io_wally::dispatch
{
    class tx_in_flight_publications final : public std::enable_shared_from_this<tx_in_flight_publications>
    {
        friend class tx_publication;
        friend class qos1_tx_publication;
        friend class qos2_tx_publication;

       private:  // static
        static constexpr const std::uint16_t MAX_PACKET_IDENTIFIER = 0xFFFF;

       public:
        tx_in_flight_publications( const context& context,
                                   asio::io_service& io_service,
                                   std::weak_ptr<mqtt_packet_sender> sender );

        auto context( ) const -> const io_wally::context&;

        auto io_service( ) const -> asio::io_service&;

        void publish( const std::shared_ptr<protocol::publish>& incoming_publish,
                      const protocol::packet::QoS maximum_qos );

        void response_received( const std::shared_ptr<protocol::publish_ack>& publish_ack );

       private:
        auto next_packet_identifier( ) -> std::uint16_t;

        void publish_using_qos0( const std::shared_ptr<protocol::publish>& incoming_publish,
                                 const std::shared_ptr<mqtt_packet_sender>& locked_sender ) const;

        void publish_using_qos1( const std::shared_ptr<protocol::publish>& incoming_publish,
                                 const std::shared_ptr<mqtt_packet_sender>& locked_sender );

        void publish_using_qos2( const std::shared_ptr<protocol::publish>& incoming_publish,
                                 const std::shared_ptr<mqtt_packet_sender>& locked_sender );

        void release( const std::shared_ptr<tx_publication>& publication );

       private:
        const io_wally::context& context_;
        asio::io_service& io_service_;
        std::weak_ptr<mqtt_packet_sender> sender_;
        std::unordered_map<std::uint16_t, std::shared_ptr<tx_publication>> publications_{};
        std::uint16_t next_packet_identifier_{0};
    };  // class tx_in_flight_publications
}  // namespace io_wally::dispatch
