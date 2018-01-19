#pragma once

#include <string>
#include <memory>

#include <optional>
#include <boost/log/trivial.hpp>

#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"

namespace framework
{
    class packet_sender_mock : public io_wally::mqtt_packet_sender,
                               public std::enable_shared_from_this<packet_sender_mock>
    {
       public:  // static
        static io_wally::mqtt_packet_sender::ptr create( )
        {
            return std::make_shared<packet_sender_mock>( );
        }

       public:
        /// \brief Return ID of connected client, if already authenticated. Otherwise, return \c std::nullopt.
        virtual const std::optional<const std::string>& client_id( ) const override
        {
            return client_id_;
        }

        /// \brief Send an \c mqtt_packet to connected client.
        virtual void send( io_wally::protocol::mqtt_packet::ptr /* packet */ ) override
        {
        }

        /// \brief Stop this connection, closing its \c tcp::socket.
        virtual void stop( const std::string& /* /message */,
                           const boost::log::trivial::severity_level /* log_level */ ) override
        {
        }

        /// \brief Return a string representation to be used in log output.
        ///
        /// \return A string representation to be used in log output
        virtual operator const std::string&( ) const override
        {
            return to_string_;
        }

       private:
        const std::optional<const std::string> client_id_{"mock_client"};
        const std::string to_string_{"mock_packet_sender"};
    };
}
