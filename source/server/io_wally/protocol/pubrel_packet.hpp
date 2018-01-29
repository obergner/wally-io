#pragma once

#include <cstdint>
#include <sstream>
#include <vector>

#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_ack_packet.hpp"

namespace io_wally
{
    namespace protocol
    {
        /// \brief PUBREL packet, sent in response to a \c PUBLISH packet (iff QoS > 0).
        ///
        /// A PUBREL packet contains in its variable header the \c packet_identifier sent by a client in the
        /// corresponding PUBLISH packet.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718068
        struct pubrel final : public publish_ack
        {
           public:
            pubrel( const uint16_t packet_identifier ) : publish_ack{packet::Type::PUBREL, packet_identifier, 0x02}
            {
            }

            /// \return A string representation to be used in log output
            virtual const std::string to_string( ) const override
            {
                auto output = std::ostringstream{};
                output << "pubrel[pktid:" << packet_identifier_ << "]";

                return output.str( );
            }
        };  // struct pubrel

    }  // namespace protocol
}  // namespace io_wally
