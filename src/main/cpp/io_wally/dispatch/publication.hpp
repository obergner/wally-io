#pragma once

#include <cstdint>
#include <memory>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

#include "io_wally/context.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/puback_packet.hpp"

namespace io_wally
{
    namespace dispatch
    {
        class in_flight_publications;

        class publication
        {
           public:
            uint16_t packet_identifier( ) const;

            virtual void start( std::shared_ptr<mqtt_packet_sender> sender ) = 0;

            virtual void response_received( std::shared_ptr<const protocol::mqtt_ack> ack,
                                            std::shared_ptr<mqtt_packet_sender> sender ) = 0;

           protected:
            publication( in_flight_publications& parent, std::shared_ptr<const protocol::publish> publish );

           protected:
            in_flight_publications& parent_;
            const std::uint32_t ack_timeout_ms_;
            const std::size_t max_retries_;
            boost::asio::io_service::strand strand_;
            boost::asio::steady_timer retry_on_timeout_;
            std::shared_ptr<const protocol::publish> publish_;
            std::uint16_t retry_count_{0};
        };  // class publication

        class qos1_publication final : public publication, public std::enable_shared_from_this<qos1_publication>
        {
           private:
            enum class state : std::uint8_t
            {
                initial = 0,

                waiting_for_ack = 1,

                completed = 2,

                terminally_failed = 3
            };

           public:
            qos1_publication( in_flight_publications& parent, std::shared_ptr<const protocol::publish> publish );

            virtual void start( std::shared_ptr<mqtt_packet_sender> sender ) override;

            virtual void response_received( std::shared_ptr<const protocol::mqtt_ack> ack,
                                            std::shared_ptr<mqtt_packet_sender> /* sender */ ) override;

           private:
            void ack_timeout_expired( std::shared_ptr<mqtt_packet_sender> sender );

            void start_ack_timeout( std::shared_ptr<mqtt_packet_sender> sender );

           private:
            state state_{state::initial};
        };  // class qos1_publication
    }       // namespace dispatch
}  // namespace io_wally
