#pragma once

#include <string>
#include <memory>

#include <boost/optional.hpp>
#include <boost/asio_queue.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/dispatch/common.hpp"
#include "io_wally/protocol/common.hpp"

namespace io_wally
{
    /// \brief "Lightweigth" \c mqtt_connection interface targetting client code that does not need to and should not
    /// have access to a full-blown \c mqtt_connection object.
    class mqtt_packet_sender
    {
       public:  // static
        /// Shared pointer
        using ptr = std::shared_ptr<mqtt_packet_sender>;

        /// A container for protocol packets
        using packet_container_t = dispatch::packet_container<mqtt_packet_sender>;

        /// Queue of protocol packet containers
        using packetq_t = boost::asio::simple_queue<packet_container_t::ptr>;

       public:
        /// \brief Return ID of connected client, if already authenticated. Otherwise, return \c boost::none.
        virtual const boost::optional<const std::string>& client_id( ) const = 0;

        /// \brief Send an \c mqtt_packet to connected client.
        virtual void send( protocol::mqtt_packet::ptr packet ) = 0;

        /// \brief Stop this connection, closing its \c tcp::socket.
        virtual void stop( const std::string& message = "",
                           const boost::log::trivial::severity_level log_level = boost::log::trivial::info ) = 0;

        /// \brief Return a string representation to be used in log output.
        ///
        /// \return A string representation to be used in log output
        virtual const std::string to_string( ) const = 0;

        inline friend std::ostream& operator<<( std::ostream& output, mqtt_packet_sender const& packet_sender )
        {
            output << packet_sender.to_string( );

            return output;
        }
    };  // class mqtt_packet_sender
}  // namespace io_wally
