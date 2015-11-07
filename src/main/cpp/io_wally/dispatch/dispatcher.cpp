#include "io_wally/dispatch/dispatcher.hpp"

#include <cassert>
#include <string>
#include <memory>

#include <boost/asio.hpp>

#include "boost/asio_queue.hpp"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/connect_packet.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/protocol/puback_packet.hpp"
#include "io_wally/dispatch/common.hpp"

namespace io_wally
{
    namespace dispatch
    {
        using lvl = boost::log::trivial::severity_level;

        // ------------------------------------------------------------------------------------------------------------
        // Public
        // ------------------------------------------------------------------------------------------------------------

        dispatcher::ptr dispatcher::create( const context& context, mqtt_packet_sender::packetq_t& dispatchq )
        {
            return std::make_shared<dispatcher>( context, dispatchq );
        }

        dispatcher::dispatcher( const context& context, mqtt_packet_sender::packetq_t& dispatchq )
            : context_{context}, dispatchq_{dispatchq}
        {
        }

        void dispatcher::run( )
        {
            BOOST_LOG_SEV( logger_, lvl::info ) << "START:   Dispatcher ...";

            do_receive_packet( );

            dispatcher_pool_.run( );
            BOOST_LOG_SEV( logger_, lvl::info ) << "STARTED: Dispatcher";
        }

        void dispatcher::stop( const std::string& message )
        {
            BOOST_LOG_SEV( logger_, lvl::info ) << "STOPPING: Dispatcher (" << message << ") ...";
            session_manager_.destroy_all( );
            dispatcher_pool_.stop( );
            BOOST_LOG_SEV( logger_, lvl::info ) << "STOPPED:  Dispatcher (" << message << ")";
        }

        // ------------------------------------------------------------------------------------------------------------
        // Private
        // ------------------------------------------------------------------------------------------------------------

        void dispatcher::do_receive_packet( )
        {
            auto self = shared_from_this( );
            packet_receiver_.async_deq( strand_.wrap( [self](
                const boost::system::error_code& ec, mqtt_packet_sender::packet_container_t::ptr packet_container )
                                                      {
                                                          self->handle_packet_received( ec, packet_container );
                                                      } ) );
        }

        void dispatcher::handle_packet_received( const boost::system::error_code& ec,
                                                 mqtt_packet_sender::packet_container_t::ptr packet_container )
        {
            if ( ec )
            {
                BOOST_LOG_SEV( logger_, lvl::error ) << "RX FAILED: [ec:" << ec << "|emsg:" << ec.message( ) << "]";
            }
            else
            {
                BOOST_LOG_SEV( logger_, lvl::debug ) << "RX: " << *packet_container->packet( );
                if ( packet_container->packet_type( ) == protocol::packet::Type::CONNECT )
                {
                    session_manager_.client_connected( packet_container->client_id( ),
                                                       packet_container->rx_connection( ) );
                }
                else if ( packet_container->packet_type( ) == protocol::packet::Type::DISCONNECT )
                {
                    session_manager_.client_disconnected( packet_container->client_id( ),
                                                          packet_container->disconnect_reason( ) );
                }
                else if ( packet_container->packet_type( ) == protocol::packet::Type::SUBSCRIBE )
                {
                    auto subscribe = packet_container->packetAs<protocol::subscribe>( );
                    session_manager_.client_subscribed( packet_container->client_id( ), subscribe );
                }
                else if ( packet_container->packet_type( ) == protocol::packet::Type::PUBLISH )
                {
                    auto publish = packet_container->packetAs<protocol::publish>( );
                    // For now, we only support QoS 0 and QoS 1
                    assert( publish->header( ).flags( ).qos( ) != protocol::packet::QoS::EXACTLY_ONCE );

                    session_manager_.client_published( packet_container->client_id( ), publish );
                }
                else if ( packet_container->packet_type( ) == protocol::packet::Type::PUBACK )
                {
                    auto puback = packet_container->packetAs<protocol::puback>( );
                    session_manager_.client_acked_publish( packet_container->client_id( ), puback );
                }
                else
                {
                    assert( false );
                }
            }
            do_receive_packet( );
        }
    }  // namespace dispatch
}  // namespace io_wally
