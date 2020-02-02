#pragma once

#include <cstdint>
#include <sstream>
#include <vector>

#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_ack_packet.hpp"

namespace io_wally::protocol
{
    /// \brief PUBACK packet, sent in response to a \c PUBLISH packet (iff QoS > 0).
    ///
    /// A PUBACK packet contains in its variable header the \c packet_identifier sent by a client in the
    /// corresponding PUBLISH packet.
    ///
    /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718068
    struct puback final : public publish_ack
    {
       public:
        puback( const uint16_t packet_identifier ) : publish_ack{packet::Type::PUBACK, packet_identifier}
        {
        }

        /// \return A string representation to be used in log output
        [[nodiscard]] auto to_string( ) const -> const std::string override
        {
            auto output = std::ostringstream{};
            output << "puback[pktid:" << packet_identifier_ << "]";

            return output.str( );
        }
    };  // struct puback

}  // namespace io_wally::protocol
