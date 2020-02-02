#pragma once

#include <memory>
#include <optional>
#include <ostream>
#include <string>

#include <spdlog/spdlog.h>

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

       public:
        virtual ~mqtt_packet_sender( ) = default;

        /// \brief Return ID of connected client, if already authenticated. Otherwise, return \c std::nullopt.
        [[nodiscard]] virtual auto client_id( ) const -> const std::optional<const std::string>& = 0;

        /// \brief Send an \c mqtt_packet to connected client.
        virtual void send( protocol::mqtt_packet::ptr packet ) = 0;

        /// \brief Stop this connection, closing its \c tcp::socket.
        virtual void stop( const std::string& message = "",
                           const spdlog::level::level_enum log_level = spdlog::level::level_enum::info ) = 0;

        /// \brief Return a string representation to be used in log output.
        ///
        /// \return A string representation to be used in log output
        virtual operator const std::string&( ) const = 0;

        inline friend auto operator<<( std::ostream& output, mqtt_packet_sender const& packet_sender ) -> std::ostream&
        {
            output << static_cast<const std::string&>( packet_sender );

            return output;
        }
    };  // class mqtt_packet_sender
}  // namespace io_wally
