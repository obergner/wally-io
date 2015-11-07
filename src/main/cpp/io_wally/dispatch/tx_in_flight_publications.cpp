#include "io_wally/dispatch/tx_in_flight_publications.hpp"

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
        // ------------------------------------------------------------------------------------------------------------
        //  Public
        // ------------------------------------------------------------------------------------------------------------

        tx_in_flight_publications::tx_in_flight_publications( const io_wally::context& context,
                                                              boost::asio::io_service& io_service,
                                                              std::weak_ptr<mqtt_packet_sender> sender )
            : context_{context}, io_service_{io_service}, sender_{sender}
        {
        }

        const io_wally::context& tx_in_flight_publications::context( ) const
        {
            return context_;
        }

        boost::asio::io_service& tx_in_flight_publications::io_service( ) const
        {
            return io_service_;
        }

        void tx_in_flight_publications::publish_received( std::shared_ptr<protocol::publish> incoming_publish )
        {
            auto const qos = incoming_publish->header( ).flags( ).qos( );
            assert( ( qos == protocol::packet::QoS::AT_LEAST_ONCE ) || ( qos == protocol::packet::QoS::AT_MOST_ONCE ) );

            auto locked_sender = sender_.lock( );
            if ( !locked_sender )
            {
                // Client connection has been closed. Nothing we can do right now.
                return;
            }

            if ( qos == protocol::packet::QoS::AT_MOST_ONCE )
            {
                locked_sender->send( incoming_publish );
            }
            else if ( qos == protocol::packet::QoS::AT_LEAST_ONCE )
            {
                qos1_publish_received( incoming_publish, locked_sender );
            }
        }

        void tx_in_flight_publications::response_received( std::shared_ptr<protocol::mqtt_ack> ack )
        {
            const protocol::packet::Type ack_type = ack->header( ).type( );
            assert( ack_type == protocol::packet::Type::PUBACK );

            auto locked_sender = sender_.lock( );
            if ( !locked_sender )
            {
                // Client connection has gone away. The client session we belong to will likely be discarded soon.
                return;
            }

            if ( ack_type == protocol::packet::Type::PUBACK )
            {
                auto puback = std::dynamic_pointer_cast<protocol::puback>( ack );
                puback_received( puback, locked_sender );
            }
        }

        // ------------------------------------------------------------------------------------------------------------
        //  Private
        // ------------------------------------------------------------------------------------------------------------

        std::uint16_t tx_in_flight_publications::next_packet_identifier( )
        {
            // For now, we don't need to be thread-safe
            if ( next_packet_identifier_ == MAX_PACKET_IDENTIFIER )
            {
                next_packet_identifier_ = 0;
            }
            else
            {
                ++next_packet_identifier_;
            }
            return next_packet_identifier_;
        }

        void tx_in_flight_publications::qos1_publish_received( std::shared_ptr<protocol::publish> incoming_publish,
                                                               std::shared_ptr<mqtt_packet_sender> locked_sender )
        {
            // Need to copy incoming PUBLISH packet since we now start a new, unrelated OUTGOING publication we need a
            // packet identifier for that is unique for THIS client, not the client that sent this incoming PUBLISH.
            auto outgoing_publish = incoming_publish->with_new_packet_identifier( next_packet_identifier( ) );

            auto publish_itr = std::unordered_map<std::uint16_t, std::shared_ptr<publication>>::iterator{};
            auto inserted = false;
            std::tie( publish_itr, inserted ) = publications_.emplace(
                std::make_pair( outgoing_publish->packet_identifier( ),
                                std::make_shared<qos1_publication>( *this, outgoing_publish ) ) );

            assert( inserted );  // Could only happen if we have more than 65535 in flight publications

            ( *publish_itr ).second->start( locked_sender );
        }

        void tx_in_flight_publications::puback_received( std::shared_ptr<protocol::puback> puback,
                                                         std::shared_ptr<mqtt_packet_sender> locked_sender )
        {
            auto const pktid = puback->packet_identifier( );
            if ( publications_.count( pktid ) > 0 )
            {
                publications_[pktid]->response_received( puback, locked_sender );
            }
            else
            {
                // TODO: what do we do if we receive a puback for a publish we don't know about?
            }
        }

        void tx_in_flight_publications::qos2_publish_received( std::shared_ptr<protocol::publish> incoming_publish,
                                                               std::shared_ptr<mqtt_packet_sender> locked_sender )
        {
            // Need to copy incoming PUBLISH packet since we now start a new, unrelated OUTGOING publication we need a
            // packet identifier for that is unique for THIS client, not the client that sent this incoming PUBLISH.
            auto outgoing_publish = incoming_publish->with_new_packet_identifier( next_packet_identifier( ) );

            auto publish_itr = std::unordered_map<std::uint16_t, std::shared_ptr<publication>>::iterator{};
            auto inserted = false;
            std::tie( publish_itr, inserted ) = publications_.emplace(
                std::make_pair( outgoing_publish->packet_identifier( ),
                                std::make_shared<qos2_publication>( *this, outgoing_publish ) ) );

            assert( inserted );  // Could only happen if we have more than 65535 in flight publications

            ( *publish_itr ).second->start( locked_sender );
        }

        void tx_in_flight_publications::pubrec_received( std::shared_ptr<protocol::pubrec> puback,
                                                         std::shared_ptr<mqtt_packet_sender> locked_sender )
        {
            auto const pktid = puback->packet_identifier( );
            if ( publications_.count( pktid ) > 0 )
            {
                publications_[pktid]->response_received( puback, locked_sender );
            }
            else
            {
                // TODO: what do we do if we receive a puback for a publish we don't know about?
            }
        }

        void tx_in_flight_publications::pubcomp_received( std::shared_ptr<protocol::pubcomp> puback,
                                                          std::shared_ptr<mqtt_packet_sender> locked_sender )
        {
            auto const pktid = puback->packet_identifier( );
            if ( publications_.count( pktid ) > 0 )
            {
                publications_[pktid]->response_received( puback, locked_sender );
            }
            else
            {
                // TODO: what do we do if we receive a puback for a publish we don't know about?
            }
        }

        void tx_in_flight_publications::release( std::shared_ptr<publication> publication )
        {
            auto const erase_count = publications_.erase( publication->packet_identifier( ) );
            assert( erase_count == 1 );
        }
    }  // namespace dispatch
}  // namespace io_wally
