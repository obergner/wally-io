#pragma once

#include <cstdint>
#include <memory>
#include <chrono>

#include <asio.hpp>
#include <asio/steady_timer.hpp>

#include "io_wally/context.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/pubrel_packet.hpp"

namespace io_wally
{
    namespace dispatch
    {
        class rx_in_flight_publications;

        class rx_publication : public std::enable_shared_from_this<rx_publication>
        {
           private:
            enum class state : std::uint8_t
            {
                initial = 0,

                waiting_for_rel = 1,

                completed = 2,

                terminally_failed = 3
            };

           public:
            rx_publication( rx_in_flight_publications& parent, std::shared_ptr<protocol::publish> publish );

            uint16_t packet_identifier( ) const;

            void start( std::shared_ptr<mqtt_packet_sender> sender );

            void client_sent_pubrel( std::shared_ptr<protocol::pubrel> pubrel,
                                     std::shared_ptr<mqtt_packet_sender> sender );

           private:
            void start_pubrel_timeout( std::shared_ptr<mqtt_packet_sender> sender );

            void pubrel_timeout_expired( std::shared_ptr<mqtt_packet_sender> sender );

           private:
            state state_{state::initial};
            rx_in_flight_publications& parent_;
            const std::uint32_t pubrel_timeout_ms_;
            const std::size_t max_retries_;
            ::asio::io_service::strand strand_;
            ::asio::steady_timer retry_on_timeout_;
            const std::uint16_t publish_id_;
            std::uint16_t retry_count_{0};
        };  // class rx_publication
    }       // namespace dispatch
}  // namespace io_wally
