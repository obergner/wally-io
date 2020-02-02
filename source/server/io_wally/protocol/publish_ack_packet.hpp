#pragma once

#include <cstdint>
#include <sstream>
#include <vector>

#include "io_wally/protocol/common.hpp"

namespace io_wally::protocol
{
    /// \brief Abstract base class for PUBACK, PUBREC, PUBREL, PUBCOMP
    ///
    /// All concrete publish_ack packets contain in their variable headers the \c packet_identifier sent by a client
    /// in the corresponding PUBLISH packet.
    ///
    /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718068
    struct publish_ack : public mqtt_ack
    {
       public:
        /// \brief Return \c packet_identifier sent in corresponding PUBLISH packet.
        ///
        /// \return \c packet_identifier sent in corresponding PUBLISH packet.
        [[nodiscard]] auto packet_identifier( ) const -> uint16_t
        {
            return packet_identifier_;
        }

        /// \return A string representation to be used in log output
        [[nodiscard]] auto to_string( ) const -> const std::string override = 0;

       protected:
        publish_ack( const packet::Type type, const uint16_t packet_identifier, const uint8_t flags = 0x00 )
            : mqtt_ack{type, 2, flags}, packet_identifier_{packet_identifier}
        {
        }

       protected:
        const uint16_t packet_identifier_;
    };  // struct publish_ack

}  // namespace io_wally::protocol
