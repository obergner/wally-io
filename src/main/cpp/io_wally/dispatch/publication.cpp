#include "io_wally/dispatch/publication.hpp"

#include <cstdint>
#include <memory>
#include <chrono>

#include "io_wally/context.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/puback_packet.hpp"
#include "io_wally/dispatch/tx_in_flight_publications.hpp"

namespace io_wally
{
    namespace dispatch
    {
        // ------------------------------------------------------------------------------------------------------------
        // class: publication
        // ------------------------------------------------------------------------------------------------------------

        uint16_t publication::packet_identifier( ) const
        {
            return publish_->packet_identifier( );
        }

        publication::publication( tx_in_flight_publications& parent, std::shared_ptr<const protocol::publish> publish )
            : parent_{parent},
              ack_timeout_ms_{parent.context( )[io_wally::context::PUB_ACK_TIMEOUT].as<const std::uint32_t>( )},
              max_retries_{parent.context( )[io_wally::context::PUB_MAX_RETRIES].as<const std::size_t>( )},
              strand_{parent.io_service( )},
              retry_on_timeout_{parent.io_service( )},
              publish_{publish}
        {
        }

        // ------------------------------------------------------------------------------------------------------------
        // class: qos1_publication
        // ------------------------------------------------------------------------------------------------------------

        qos1_publication::qos1_publication( tx_in_flight_publications& parent,
                                            std::shared_ptr<const protocol::publish> publish )
            : publication( parent, publish )
        {
        }

        void qos1_publication::start( std::shared_ptr<mqtt_packet_sender> sender )
        {
            sender->send( publish_ );
            start_ack_timeout( sender );
        }

        void qos1_publication::response_received( std::shared_ptr<const protocol::mqtt_ack> ack,
                                                  std::shared_ptr<mqtt_packet_sender> /* sender */ )
        {
            assert( state_ == state::waiting_for_ack );
            assert( ack->header( ).type( ) == protocol::packet::Type::PUBACK );

            auto pub_ack = std::dynamic_pointer_cast<const protocol::puback>( ack );
            assert( pub_ack->packet_identifier( ) == publish_->packet_identifier( ) );

            state_ = state::completed;
            retry_on_timeout_.cancel( );

            parent_.release( shared_from_this( ) );
        }

        void qos1_publication::ack_timeout_expired( std::shared_ptr<mqtt_packet_sender> sender )
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
                parent_.release( shared_from_this( ) );
            }
        }

        void qos1_publication::start_ack_timeout( std::shared_ptr<mqtt_packet_sender> sender )
        {
            state_ = state::waiting_for_ack;

            auto self = shared_from_this( );
            auto ack_tmo = std::chrono::milliseconds{ack_timeout_ms_};
            retry_on_timeout_.expires_from_now( ack_tmo );
            retry_on_timeout_.async_wait( strand_.wrap( [self, sender]( const boost::system::error_code& ec )
                                                        {
                                                            if ( !ec )
                                                            {
                                                                self->ack_timeout_expired( sender );
                                                            }
                                                        } ) );
        }
    }  // namespace dispatch
}  // namespace io_wally
