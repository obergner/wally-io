#pragma once

#include <cstdint>
#include <memory>
#include <chrono>

#include "asio.hpp"
#include "asio/steady_timer.hpp"

#include "io_wally/context.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/publish_ack_packet.hpp"

namespace io_wally
{
    namespace dispatch
    {
        class tx_in_flight_publications;

        class tx_publication
        {
           public:
            virtual ~tx_publication( )
            {
            }

            uint16_t packet_identifier( ) const;

            virtual void start( std::shared_ptr<mqtt_packet_sender> sender ) = 0;

            virtual void response_received( std::shared_ptr<protocol::publish_ack> ack,
                                            std::shared_ptr<mqtt_packet_sender> sender ) = 0;

           protected:
            tx_publication( tx_in_flight_publications& parent, std::shared_ptr<protocol::publish> publish );

           protected:
            tx_in_flight_publications& parent_;
            const std::uint32_t ack_timeout_ms_;
            const std::size_t max_retries_;
            ::asio::io_service::strand strand_;
            ::asio::steady_timer retry_on_timeout_;
            std::shared_ptr<protocol::publish> publish_;
            std::uint16_t retry_count_{0};
        };  // class tx_publication

        class qos1_tx_publication final : public tx_publication,
                                          public std::enable_shared_from_this<qos1_tx_publication>
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
            qos1_tx_publication( tx_in_flight_publications& parent, std::shared_ptr<protocol::publish> publish );

            ~qos1_tx_publication( )
            {
                retry_on_timeout_.cancel( );
            }

            virtual void start( std::shared_ptr<mqtt_packet_sender> sender ) override;

            virtual void response_received( std::shared_ptr<protocol::publish_ack> ack,
                                            std::shared_ptr<mqtt_packet_sender> /* sender */ ) override;

           private:
            void ack_timeout_expired( std::shared_ptr<mqtt_packet_sender> sender );

            void start_ack_timeout( std::shared_ptr<mqtt_packet_sender> sender );

           private:
            state state_{state::initial};
        };  // class qos1_tx_publication

        class qos2_tx_publication final : public tx_publication,
                                          public std::enable_shared_from_this<qos2_tx_publication>
        {
           private:
            enum class state : std::uint8_t
            {
                initial = 0,

                waiting_for_rec = 1,

                waiting_for_comp = 2,

                completed = 3,

                terminally_failed = 4
            };

           public:
            qos2_tx_publication( tx_in_flight_publications& parent, std::shared_ptr<protocol::publish> publish );

            ~qos2_tx_publication( )
            {
                retry_on_timeout_.cancel( );
            }

            virtual void start( std::shared_ptr<mqtt_packet_sender> sender ) override;

            virtual void response_received( std::shared_ptr<protocol::publish_ack> ack,
                                            std::shared_ptr<mqtt_packet_sender> /* sender */ ) override;

           private:
            void ack_timeout_expired( std::shared_ptr<mqtt_packet_sender> sender );

            void start_ack_timeout( std::shared_ptr<mqtt_packet_sender> sender );

           private:
            state state_{state::initial};
        };  // class qos1_tx_publication
    }       // namespace dispatch
}  // namespace io_wally
