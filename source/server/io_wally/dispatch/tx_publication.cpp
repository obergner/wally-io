#include "io_wally/dispatch/tx_publication.hpp"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <memory>

#include <spdlog/spdlog.h>

#include "io_wally/context.hpp"
#include "io_wally/dispatch/tx_in_flight_publications.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/puback_packet.hpp"
#include "io_wally/protocol/pubcomp_packet.hpp"
#include "io_wally/protocol/publish_ack_packet.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/pubrel_packet.hpp"

namespace io_wally
{
    namespace dispatch
    {
        // ------------------------------------------------------------------------------------------------------------
        // class: tx_publication
        // ------------------------------------------------------------------------------------------------------------

        uint16_t tx_publication::packet_identifier( ) const
        {
            return publish_->packet_identifier( );
        }

        tx_publication::tx_publication( tx_in_flight_publications& parent, std::shared_ptr<protocol::publish> publish )
            : parent_{parent},
              ack_timeout_ms_{parent.context( )[io_wally::context::PUB_ACK_TIMEOUT].as<std::uint32_t>( )},
              max_retries_{parent.context( )[io_wally::context::PUB_MAX_RETRIES].as<std::size_t>( )},
              strand_{parent.io_service( )},
              retry_on_timeout_{parent.io_service( )},
              publish_{publish}
        {
        }

        // ------------------------------------------------------------------------------------------------------------
        // class: qos1_tx_publication
        // ------------------------------------------------------------------------------------------------------------

        qos1_tx_publication::qos1_tx_publication( tx_in_flight_publications& parent,
                                                  std::shared_ptr<protocol::publish> publish )
            : tx_publication( parent, publish )
        {
            publish_->qos( protocol::packet::QoS::AT_LEAST_ONCE );
        }

        void qos1_tx_publication::start( std::shared_ptr<mqtt_packet_sender> sender )
        {
            sender->send( publish_ );
            start_ack_timeout( sender );
        }

        void qos1_tx_publication::response_received( std::shared_ptr<protocol::publish_ack> ack,
                                                     std::shared_ptr<mqtt_packet_sender> /* sender */ )
        {
            assert( state_ == state::waiting_for_ack );
            assert( ack->header( ).type( ) == protocol::packet::Type::PUBACK );

            const auto pub_ack = std::dynamic_pointer_cast<protocol::puback>( ack );
            assert( pub_ack->packet_identifier( ) == publish_->packet_identifier( ) );

            state_ = state::completed;
            retry_on_timeout_.cancel( );
            // Now, this packet identifiere may be re-used
            parent_.release( shared_from_this( ) );
        }

        void qos1_tx_publication::ack_timeout_expired( std::shared_ptr<mqtt_packet_sender> sender )
        {
            assert( state_ == state::waiting_for_ack );
            if ( ++retry_count_ <= max_retries_ )
            {
                sender->send( publish_ );
                start_ack_timeout( sender );
            }
            else
            {
                state_ = state::terminally_failed;
                // Now, this packet identifiere may be re-used
                parent_.release( shared_from_this( ) );
            }
        }

        void qos1_tx_publication::start_ack_timeout( std::shared_ptr<mqtt_packet_sender> sender )
        {
            state_ = state::waiting_for_ack;

            // We use a weak_ptr in this async callback since our owner - mqtt_client_session via
            // tx_in_flight_publications - may go away anytime while we wait for this timeout to expire
            const auto self_weak = std::weak_ptr<qos1_tx_publication>{shared_from_this( )};
            const auto ack_tmo = std::chrono::milliseconds{ack_timeout_ms_};
            retry_on_timeout_.expires_from_now( ack_tmo );
            retry_on_timeout_.async_wait( strand_.wrap( [self_weak, sender]( const std::error_code& ec ) {
                if ( ec )
                {
                    return;
                }
                if ( const auto self = self_weak.lock( ) )
                {
                    self->ack_timeout_expired( sender );
                }
            } ) );
        }

        // ------------------------------------------------------------------------------------------------------------
        // class: qos2_tx_publication
        // ------------------------------------------------------------------------------------------------------------

        qos2_tx_publication::qos2_tx_publication( tx_in_flight_publications& parent,
                                                  std::shared_ptr<protocol::publish> publish )
            : tx_publication( parent, publish )
        {
            publish_->qos( protocol::packet::QoS::EXACTLY_ONCE );
        }

        void qos2_tx_publication::start( std::shared_ptr<mqtt_packet_sender> sender )
        {
            sender->send( publish_ );
            start_ack_timeout( sender );
        }

        void qos2_tx_publication::response_received( std::shared_ptr<protocol::publish_ack> ack,
                                                     std::shared_ptr<mqtt_packet_sender> sender )
        {
            assert( ( state_ == state::waiting_for_rec ) || ( state_ == state::waiting_for_comp ) );

            retry_on_timeout_.cancel( );
            if ( state_ == state::waiting_for_rec )
            {
                if ( ack->header( ).type( ) != protocol::packet::Type::PUBREC )
                {
                    // Protocol violation: client did not send PUBREC but one of PUBCOMP or even PUBACK
                    sender->stop(
                        "Protocol violation: client did not send expected PUBREC but one of PUBCOMP or PUBACK",
                        spdlog::level::level_enum::warn );
                }
                else
                {
                    const auto pubrec = std::dynamic_pointer_cast<protocol::pubrec>( ack );
                    assert( pubrec->packet_identifier( ) == publish_->packet_identifier( ) );

                    const auto pubrel = std::make_shared<protocol::pubrel>( publish_->packet_identifier( ) );
                    sender->send( pubrel );

                    start_ack_timeout( sender );
                }
            }
            else  // waiting for pubcomp
            {
                if ( ack->header( ).type( ) == protocol::packet::Type::PUBREC )
                {
                    // Client re-sent PUBREC. This likely means it did not receive our PUBREL. Let's sent it again.
                    const auto pubrel = std::make_shared<protocol::pubrel>( publish_->packet_identifier( ) );
                    sender->send( pubrel );

                    start_ack_timeout( sender );
                }
                else
                {
                    assert( ack->header( ).type( ) == protocol::packet::Type::PUBCOMP );
                    state_ = state::completed;
                    // Now, this packet identifier may be re-used
                    parent_.release( shared_from_this( ) );
                }
            }
        }

        void qos2_tx_publication::ack_timeout_expired( std::shared_ptr<mqtt_packet_sender> sender )
        {
            assert( ( state_ == state::waiting_for_rec ) || ( state_ == state::waiting_for_comp ) );
            if ( ++retry_count_ <= max_retries_ )
            {
                if ( state_ == state::waiting_for_rec )
                {
                    publish_->dup( true );  // Mark this a duplication publish
                    sender->send( publish_ );
                }
                else  // waiting for pubcomp
                {
                    const auto pubrel = std::make_shared<protocol::pubrel>( publish_->packet_identifier( ) );
                    sender->send( pubrel );
                }
                start_ack_timeout( sender );
            }
            else
            {
                state_ = state::terminally_failed;
                // Now, this packet identifier may be re-used
                parent_.release( shared_from_this( ) );
            }
        }

        void qos2_tx_publication::start_ack_timeout( std::shared_ptr<mqtt_packet_sender> sender )
        {
            if ( state_ == state::initial )
            {
                state_ = state::waiting_for_rec;
            }
            else
            {
                state_ = state::waiting_for_comp;
            }

            // We use a weak_ptr in this async callback since our owner - mqtt_client_session via
            // tx_in_flight_publications - may go away anytime while we wait for this timeout to expire
            const auto self_weak = std::weak_ptr<qos2_tx_publication>{shared_from_this( )};
            const auto ack_tmo = std::chrono::milliseconds{ack_timeout_ms_};
            retry_on_timeout_.expires_from_now( ack_tmo );
            retry_on_timeout_.async_wait( strand_.wrap( [self_weak, sender]( const std::error_code& ec ) {
                if ( ec )
                {
                    return;
                }
                if ( const auto self = self_weak.lock( ) )
                {
                    self->ack_timeout_expired( sender );
                }
            } ) );
        }
    }  // namespace dispatch
}  // namespace io_wally
