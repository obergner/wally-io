#include "io_wally/dispatch/rx_in_flight_publications.hpp"

#include <cassert>
#include <cstdint>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <utility>

#include <boost/asio.hpp>

#include "io_wally/context.hpp"
#include "io_wally/dispatch/rx_publication.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/pubrel_packet.hpp"

namespace io_wally
{
    namespace dispatch
    {
        // ------------------------------------------------------------------------------------------------------------
        //  Public
        // ------------------------------------------------------------------------------------------------------------

        rx_in_flight_publications::rx_in_flight_publications( const io_wally::context& context,
                                                              boost::asio::io_service& io_service,
                                                              std::weak_ptr<mqtt_packet_sender> sender )
            : context_{context}, io_service_{io_service}, sender_{sender}
        {
        }

        const io_wally::context& rx_in_flight_publications::context( ) const
        {
            return context_;
        }

        boost::asio::io_service& rx_in_flight_publications::io_service( ) const
        {
            return io_service_;
        }

        bool rx_in_flight_publications::client_sent_publish( std::shared_ptr<protocol::publish> incoming_publish )
        {
            if ( publications_.count( incoming_publish->packet_identifier( ) ) > 0 )
            {
                // There is still an incomplete QoS2 publication for this session using this packet identifier. MQTT
                // 3.1.1 demands to treat this publish as a client retry.
                return false;
            }

            if ( incoming_publish->qos( ) == protocol::packet::QoS::AT_LEAST_ONCE )
            {
                if ( auto locked_sender = sender_.lock( ) )
                {
                    auto puback = std::make_shared<protocol::puback>( incoming_publish->packet_identifier( ) );
                    locked_sender->send( puback );
                }
            }
            else if ( incoming_publish->qos( ) == protocol::packet::QoS::EXACTLY_ONCE )
            {
                if ( auto locked_sender = sender_.lock( ) )
                {
                    auto publish_itr = std::unordered_map<std::uint16_t, std::shared_ptr<rx_publication>>::iterator{};
                    auto inserted = false;
                    std::tie( publish_itr, inserted ) = publications_.emplace(
                        std::make_pair( incoming_publish->packet_identifier( ),
                                        std::make_shared<rx_publication>( *this, incoming_publish ) ) );

                    assert( inserted );  // Could only happen if we have more than 65535 in flight publications

                    ( *publish_itr ).second->start( locked_sender );
                }
            }
            return true;
        }

        void rx_in_flight_publications::client_sent_pubrel( std::shared_ptr<protocol::pubrel> pubrel )
        {
            auto locked_sender = sender_.lock( );
            if ( !locked_sender )
            {
                // Client connection has gone away. The client session we belong to will likely be discarded soon.
                return;
            }
            auto const pktid = pubrel->packet_identifier( );
            if ( publications_.count( pktid ) > 0 )
            {
                publications_[pktid]->client_sent_pubrel( pubrel, locked_sender );
            }
            else
            {
                locked_sender->stop(
                    "[MQTT-4.8.0-1] Protocol violation: client sent PUBREL without first sending PUBLISH",
                    boost::log::trivial::warning );
            }
        }

        // ------------------------------------------------------------------------------------------------------------
        //  Private
        // ------------------------------------------------------------------------------------------------------------

        void rx_in_flight_publications::release( std::shared_ptr<rx_publication> publication )
        {
            auto const erase_count = publications_.erase( publication->packet_identifier( ) );
            assert( erase_count == 1 );
        }
    }  // namespace dispatch
}  // namespace io_wally
