#include "io_wally/dispatch/rx_publication.hpp"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <memory>

#include "io_wally/context.hpp"
#include "io_wally/dispatch/rx_in_flight_publications.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/pubcomp_packet.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/pubrec_packet.hpp"
#include "io_wally/protocol/pubrel_packet.hpp"

namespace io_wally::dispatch
{
    // ------------------------------------------------------------------------------------------------------------
    // class: rx_publication
    // ------------------------------------------------------------------------------------------------------------

    auto rx_publication::packet_identifier( ) const -> uint16_t
    {
        return publish_id_;
    }

    rx_publication::rx_publication( rx_in_flight_publications& parent,
                                    const std::shared_ptr<protocol::publish>& publish )
        : parent_{parent},
          pubrel_timeout_ms_{parent.context( )[io_wally::context::PUB_ACK_TIMEOUT].as<std::uint32_t>( )},
          max_retries_{parent.context( )[io_wally::context::PUB_MAX_RETRIES].as<std::size_t>( )},
          strand_{parent.io_service( )},
          retry_on_timeout_{parent.io_service( )},
          publish_id_{publish->packet_identifier( )}
    {
    }

    void rx_publication::start( const std::shared_ptr<mqtt_packet_sender>& sender )
    {
        const auto pubrec = std::make_shared<protocol::pubrec>( publish_id_ );
        sender->send( pubrec );
        start_pubrel_timeout( sender );
    }

    void rx_publication::client_sent_pubrel( const std::shared_ptr<protocol::pubrel>& pubrel,
                                             const std::shared_ptr<mqtt_packet_sender>& sender )
    {
        assert( state_ == state::waiting_for_rel );
        assert( pubrel->packet_identifier( ) == publish_id_ );

        const auto pubcomp = std::make_shared<protocol::pubcomp>( publish_id_ );
        sender->send( pubcomp );

        state_ = state::completed;
        retry_on_timeout_.cancel( );
        // Now, this packet identifier may be re-used
        parent_.release( shared_from_this( ) );
    }

    void rx_publication::pubrel_timeout_expired( const std::shared_ptr<mqtt_packet_sender>& sender )
    {
        assert( state_ == state::waiting_for_rel );
        if ( ++retry_count_ <= max_retries_ )
        {
            const auto pubrec = std::make_shared<protocol::pubrec>( publish_id_ );
            sender->send( pubrec );
            start_pubrel_timeout( sender );
        }
        else
        {
            state_ = state::terminally_failed;
            // Now, this packet identifier may be re-used
            parent_.release( shared_from_this( ) );
        }
    }

    void rx_publication::start_pubrel_timeout( const std::shared_ptr<mqtt_packet_sender>& sender )
    {
        state_ = state::waiting_for_rel;

        // We use a weak_ptr in this async callback since our owner - mqtt_client_session via
        // rx_in_flight_publications - may go away anytime while we wait for this timeout to expire
        const auto self_weak = std::weak_ptr<rx_publication>{shared_from_this( )};
        const auto ack_tmo = std::chrono::milliseconds{pubrel_timeout_ms_};
        retry_on_timeout_.expires_from_now( ack_tmo );
        retry_on_timeout_.async_wait( strand_.wrap( [self_weak, sender]( const std::error_code& ec ) {
            if ( ec )
            {
                return;
            }
            if ( const auto self = self_weak.lock( ) )
            {
                self->pubrel_timeout_expired( sender );
            }
        } ) );
    }
}  // namespace io_wally::dispatch
