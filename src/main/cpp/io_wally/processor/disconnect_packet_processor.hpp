#pragma once

#include <sstream>
#include <memory>

#include <boost/asio_queue.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/processor/processor.hpp"
#include "io_wally/mqtt_connection_handle.hpp"
#include "io_wally/protocol/disconnect_packet.hpp"
#include "io_wally/dispatch/common.hpp"

namespace io_wally
{
    namespace processor
    {
        class disconnect_packet_processor : public processor<protocol::disconnect>,
                                            public std::enable_shared_from_this<disconnect_packet_processor>
        {
           private:
            using lvl = boost::log::trivial::severity_level;

           public:
            disconnect_packet_processor( const std::string& name,
                                         mqtt_connection_handle::ptr connection,
                                         boost::asio::queue_sender<mqtt_connection_handle::packetq_t>& dispatcher )
                : processor( name, connection, dispatcher )
            {
            }

           protected:
            virtual void do_process( std::shared_ptr<const protocol::disconnect> packet,
                                     const dispatch::disconnect_reason disconnect_reason ) override
            {
                dispatch_disconnect_packet( packet, disconnect_reason );
            }

           private:
            void dispatch_disconnect_packet( std::shared_ptr<const protocol::disconnect> disconnect,
                                             const dispatch::disconnect_reason disconnect_reason )
            {
                BOOST_LOG_SEV( logger_, lvl::debug ) << "--- DISPATCHING: " << *disconnect
                                                     << "[rsn:" << disconnect_reason << "] ...";
                auto connect_container = mqtt_connection_handle::packet_container_t::disconnect_packet(
                    *connection_->client_id( ), connection_, disconnect, disconnect_reason );

                auto self = shared_from_this( );
                dispatcher_.async_enq(
                    connect_container,
                    strand_.wrap( [self, disconnect, disconnect_reason]( const boost::system::error_code& ec )
                                  {
                                      self->handle_dispatch_disconnect_packet( ec, disconnect, disconnect_reason );
                                  } ) );
            }

            void handle_dispatch_disconnect_packet( const boost::system::error_code& ec,
                                                    std::shared_ptr<const protocol::disconnect> disconnect,
                                                    const dispatch::disconnect_reason disconnect_reason )
            {
                if ( ec )
                {
                    BOOST_LOG_SEV( logger_, lvl::error ) << "--- DISPATCH FAILED: " << *disconnect << " [ec:" << ec
                                                         << "|emsg:" << ec.message( ) << "]";
                }
                else
                {
                    BOOST_LOG_SEV( logger_, lvl::debug ) << "--- DISPATCHED: " << *disconnect
                                                         << " [rsn:" << disconnect_reason << "]";
                }
                auto msg = std::ostringstream{};
                msg << "--- Connection disconnected: " << disconnect_reason;
                connection_->stop( msg.str( ), lvl::info );
            }
        };  // class connect_packet_processor
    }       // namespace processor
}  // namespace io_wally
