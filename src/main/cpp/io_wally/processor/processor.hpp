#pragma once

#include <memory>

#include <boost/asio.hpp>

#include <boost/asio_queue.hpp>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/dispatch/common.hpp"
#include "io_wally/mqtt_connection_handle.hpp"
#include "io_wally/protocol/common.hpp"

namespace io_wally
{
    /// \brief Group all \c processors into a dedicated namespace. A \c processor knows how to handle MQTT packets of a
    /// specific type once they have been decoded, but *before* they are forwarded to the \c dispatcher subsystem.
    ///
    /// Main motivation for introducing the \c processor concept was to reduce ever-increasing code complexity in
    /// \c mqtt_connection.
    namespace processor
    {
        template <typename PACKET>
        class processor
        {
            static_assert( std::is_base_of<protocol::mqtt_packet, PACKET>::value,
                           "Template parameter PACKET needs to derive from protocol::mqtt_packet" );

           public:
            void process( std::shared_ptr<const PACKET> packet )
            {
                BOOST_LOG_SEV( logger_, boost::log::trivial::debug ) << "Processing: " << *packet << " ...";
                do_process( packet );
                BOOST_LOG_SEV( logger_, boost::log::trivial::info ) << "PROCESSED: " << *packet;
            }

           protected:
            processor( const std::string& name,
                       mqtt_connection_handle::ptr connection,
                       boost::asio::queue_sender<mqtt_connection_handle::packetq_t>& dispatcher )
                : name_{name}, connection_{connection}, dispatcher_{dispatcher}
            {
            }

            virtual void do_process( std::shared_ptr<const PACKET> packet,
                                     const dispatch::disconnect_reason
                                         disconnect_reason = dispatch::disconnect_reason::not_a_disconnect ) = 0;

           protected:
            const std::string name_;
            mqtt_connection_handle::ptr connection_;
            /// Queue sender for dispatching received protocol packets asynchronously (from point of view of connection)
            /// to dispatcher queue.
            boost::asio::queue_sender<mqtt_connection_handle::packetq_t>& dispatcher_;
            /// Strand
            boost::asio::io_service::strand strand_{dispatcher_.get_io_service( )};
            /// Our severity-enabled channel logger
            boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_{
                boost::log::keywords::channel = "processor:" + name_,
                boost::log::keywords::severity = boost::log::trivial::trace};
        };  // class processor
    }       // namespace processor
}  // namespace io_wally
