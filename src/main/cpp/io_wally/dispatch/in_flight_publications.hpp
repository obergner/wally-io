#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>
#include <tuple>
#include <unordered_map>

#include <boost/asio.hpp>

#include "io_wally/context.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/puback_packet.hpp"
#include "io_wally/dispatch/publication.hpp"

namespace io_wally
{
    namespace dispatch
    {
        class in_flight_publications final : public std::enable_shared_from_this<in_flight_publications>
        {
            friend class publication;
            friend class qos1_publication;

           private:  // static
            static constexpr const std::uint16_t MAX_PACKET_IDENTIFIER = 0xFFFF;

           public:
            in_flight_publications( const context& context,
                                    boost::asio::io_service& io_service,
                                    std::weak_ptr<mqtt_packet_sender> sender );

            const io_wally::context& context( ) const;

            boost::asio::io_service& io_service( ) const;

            void publish_received( std::shared_ptr<const protocol::publish> incoming_publish );

            void response_received( std::shared_ptr<const protocol::mqtt_ack> ack );

           private:
            std::uint16_t next_packet_identifier( );

            void qos1_publish_received( std::shared_ptr<const protocol::publish> incoming_publish,
                                        std::shared_ptr<mqtt_packet_sender> locked_sender );

            void puback_received( std::shared_ptr<const protocol::puback> puback,
                                  std::shared_ptr<mqtt_packet_sender> locked_sender );

            void release( std::shared_ptr<publication> publication );

           private:
            const io_wally::context& context_;
            boost::asio::io_service& io_service_;
            std::weak_ptr<mqtt_packet_sender> sender_;
            // TODO: packet identifiers are only unique per SENDER client identifier. We here are the RECEIVER's client
            // session.
            std::unordered_map<std::uint16_t, std::shared_ptr<publication>> publications_{};
            std::uint16_t next_packet_identifier_{0};
        };  // class publication
    }       // namespace dispatch
}  // namespace io_wally