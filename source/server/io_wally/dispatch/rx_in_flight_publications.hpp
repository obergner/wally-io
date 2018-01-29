#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>
#include <tuple>
#include <unordered_map>

#include <asio.hpp>

#include "io_wally/context.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/pubrel_packet.hpp"
#include "io_wally/dispatch/rx_publication.hpp"

namespace io_wally
{
    namespace dispatch
    {
        class rx_in_flight_publications final : public std::enable_shared_from_this<rx_in_flight_publications>
        {
            friend class rx_publication;

           public:
            rx_in_flight_publications( const context& context,
                                       ::asio::io_service& io_service,
                                       std::weak_ptr<mqtt_packet_sender> sender );

            const io_wally::context& context( ) const;

            ::asio::io_service& io_service( ) const;

            bool client_sent_publish( std::shared_ptr<protocol::publish> incoming_publish );

            void client_sent_pubrel( std::shared_ptr<protocol::pubrel> pubrel );

           private:
            void release( std::shared_ptr<rx_publication> publication );

           private:
            const io_wally::context& context_;
            ::asio::io_service& io_service_;
            std::weak_ptr<mqtt_packet_sender> sender_;
            std::unordered_map<std::uint16_t, std::shared_ptr<rx_publication>> publications_{};
        };  // class rx_in_flight_publications
    }       // namespace dispatch
}  // namespace io_wally
