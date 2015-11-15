#include "io_wally/dispatch/tx_in_flight_publications.hpp"

#include <iostream>
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
#include "io_wally/protocol/publish_ack_packet.hpp"
#include "io_wally/protocol/puback_packet.hpp"
#include "io_wally/protocol/pubrec_packet.hpp"
#include "io_wally/protocol/pubcomp_packet.hpp"
#include "io_wally/dispatch/tx_publication.hpp"

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

        void tx_in_flight_publications::publish( std::shared_ptr<protocol::publish> incoming_publish,
                                                 const protocol::packet::QoS maximum_qos )
        {
            auto locked_sender = sender_.lock( );
            if ( !locked_sender )
            {
                // Client connection has been closed. Nothing we can do right now.
                return;
            }

            if ( maximum_qos == protocol::packet::QoS::AT_MOST_ONCE )
            {
                publish_using_qos0( incoming_publish, locked_sender );
            }
            else if ( maximum_qos == protocol::packet::QoS::AT_LEAST_ONCE )
            {
                publish_using_qos1( incoming_publish, locked_sender );
            }
            else if ( maximum_qos == protocol::packet::QoS::EXACTLY_ONCE )
            {
                publish_using_qos2( incoming_publish, locked_sender );
            }
        }

        void tx_in_flight_publications::response_received( std::shared_ptr<protocol::publish_ack> publish_ack )
        {
            const protocol::packet::Type ack_type = publish_ack->header( ).type( );
            assert( ( ack_type == protocol::packet::Type::PUBACK ) || ( ack_type == protocol::packet::Type::PUBREC ) ||
                    ( ack_type == protocol::packet::Type::PUBCOMP ) );

            auto locked_sender = sender_.lock( );
            if ( !locked_sender )
            {
                // Client connection has gone away. The client session we belong to will likely be discarded soon.
                return;
            }
            auto const pktid = publish_ack->packet_identifier( );
            if ( publications_.count( pktid ) > 0 )
            {
                publications_[pktid]->response_received( publish_ack, locked_sender );
            }
            else
            {
                // TODO: what do we do if we receive a puback for a publish we don't know about?
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

        void tx_in_flight_publications::publish_using_qos0( std::shared_ptr<protocol::publish> incoming_publish,
                                                            std::shared_ptr<mqtt_packet_sender> locked_sender )
        {
            incoming_publish->qos( protocol::packet::QoS::AT_MOST_ONCE );
            locked_sender->send( incoming_publish );
        }

        void tx_in_flight_publications::publish_using_qos1( std::shared_ptr<protocol::publish> incoming_publish,
                                                            std::shared_ptr<mqtt_packet_sender> locked_sender )
        {
            // Need to copy incoming PUBLISH packet since we now start a new, unrelated OUTGOING publication we need a
            // packet identifier for that is unique for THIS client, not the client that sent this incoming PUBLISH.
            auto outgoing_publish = incoming_publish->with_new_packet_identifier( next_packet_identifier( ) );

            auto publish_itr = std::unordered_map<std::uint16_t, std::shared_ptr<tx_publication>>::iterator{};
            auto inserted = false;
            std::tie( publish_itr, inserted ) = publications_.emplace(
                std::make_pair( outgoing_publish->packet_identifier( ),
                                std::make_shared<qos1_tx_publication>( *this, outgoing_publish ) ) );

            assert( inserted );  // Could only happen if we have more than 65535 in flight publications

            ( *publish_itr ).second->start( locked_sender );
        }

        void tx_in_flight_publications::publish_using_qos2( std::shared_ptr<protocol::publish> incoming_publish,
                                                            std::shared_ptr<mqtt_packet_sender> locked_sender )
        {
            // Need to copy incoming PUBLISH packet since we now start a new, unrelated OUTGOING publication we need a
            // packet identifier for that is unique for THIS client, not the client that sent this incoming PUBLISH.
            auto outgoing_publish = incoming_publish->with_new_packet_identifier( next_packet_identifier( ) );

            auto publish_itr = std::unordered_map<std::uint16_t, std::shared_ptr<tx_publication>>::iterator{};
            auto inserted = false;
            std::tie( publish_itr, inserted ) = publications_.emplace(
                std::make_pair( outgoing_publish->packet_identifier( ),
                                std::make_shared<qos2_tx_publication>( *this, outgoing_publish ) ) );

            assert( inserted );  // Could only happen if we have more than 65535 in flight publications

            ( *publish_itr ).second->start( locked_sender );
        }

        void tx_in_flight_publications::release( std::shared_ptr<tx_publication> publication )
        {
            std::cerr << "release: " << publication->packet_identifier( ) << std::endl << std::flush;
            auto const erase_count = publications_.erase( publication->packet_identifier( ) );
            std::cerr << "release: " << erase_count << std::endl << std::flush;
            assert( erase_count == 1 );
        }
    }  // namespace dispatch
}  // namespace io_wally
