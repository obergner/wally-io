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

           public:
            in_flight_publications( const context& context,
                                    boost::asio::io_service& io_service,
                                    std::weak_ptr<mqtt_packet_sender> sender )
                : context_{context}, io_service_{io_service}, sender_{sender}
            {
            }

            const io_wally::context& context( ) const
            {
                return context_;
            }

            boost::asio::io_service& io_service( ) const
            {
                return io_service_;
            }

            void publish_received( std::shared_ptr<const protocol::publish> publish )
            {
                auto const qos = publish->header( ).flags( ).qos( );
                assert( qos == protocol::packet::QoS::AT_LEAST_ONCE );

                auto publish_itr = std::unordered_map<std::uint16_t, std::shared_ptr<publication>>::iterator{};
                auto inserted = false;
                if ( qos == protocol::packet::QoS::AT_LEAST_ONCE )
                {
                    std::tie( publish_itr, inserted ) = publications_.emplace( std::make_pair(
                        publish->packet_identifier( ), std::make_shared<qos1_publication>( *this, publish ) ) );
                }
                auto sndr = std::shared_ptr<mqtt_packet_sender>{};
                if ( inserted && ( sndr = sender_.lock( ) ) )
                {
                    ( *publish_itr ).second->start( sndr );
                }
                // TODO: what to do if client sends another PUBLISH reusing packet identifier?
            }

            void response_received( std::shared_ptr<const protocol::mqtt_ack> ack )
            {
                auto sndr = std::shared_ptr<mqtt_packet_sender>{};
                if ( !( sndr = sender_.lock( ) ) )
                {  // Client connection has gone away. The client session we belong to will likely be discarded soon.
                    return;
                }

                const protocol::packet::Type ack_type = ack->header( ).type( );
                if ( ack_type == protocol::packet::Type::PUBACK )
                {
                    auto puback = std::dynamic_pointer_cast<const protocol::puback>( ack );
                    auto const pktid = puback->packet_identifier( );
                    if ( publications_.count( pktid ) > 0 )
                    {
                        publications_[pktid]->response_received( ack, sndr );
                    }
                    else
                    {
                        // TODO: what do we do if we receive a puback for a publish we don't know about?
                    }
                }
                else
                {
                    assert( false );
                }
            }

           private:
            void release( std::shared_ptr<publication> publication )
            {
                auto const erase_count = publications_.erase( publication->packet_identifier( ) );
                assert( erase_count == 1 );
            }

           private:
            const io_wally::context& context_;
            boost::asio::io_service& io_service_;
            std::weak_ptr<mqtt_packet_sender> sender_;
            std::unordered_map<std::uint16_t, std::shared_ptr<publication>> publications_{};
        };  // class publication
    }       // namespace dispatch
}  // namespace io_wally
