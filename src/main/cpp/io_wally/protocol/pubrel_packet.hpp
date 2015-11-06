#pragma once

#include <cstdint>
#include <sstream>
#include <vector>

#include "io_wally/protocol/common.hpp"

namespace io_wally
{
    namespace protocol
    {
        /// \brief PUBREL packet, sent in response to a \c PUBLISH packet (iff QoS > 0).
        ///
        /// A PUBREL packet contains in its variable header the \c packet_identifier sent by a client in the
        /// corresponding PUBLISH packet.
        ///
        /// The PUBREL packet's payload contains a list of return codes, corresponding to the list of topic
        /// subscriptions in the acknowledged PUBLISH packet, in that order.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718068
        struct pubrel final : public mqtt_ack
        {
           public:
            // TODO: MQTT 3.1.1 mandates that bit 1 be set in PUBREL's header flags. We currently do not support
            // this, so fix.
            pubrel( const uint16_t packet_identifier )
                : mqtt_ack{packet::Type::PUBREL, 2, 0x02}, packet_identifier_{packet_identifier}
            {
            }

            /// \brief Return \c packet_identifier sent in corresponding PUBLISH packet.
            ///
            /// \return \c packet_identifier sent in corresponding PUBLISH packet.
            uint16_t packet_identifier( ) const
            {
                return packet_identifier_;
            }

            /// \return A string representation to be used in log output
            virtual const std::string to_string( ) const override
            {
                auto output = std::ostringstream{};
                output << "pubrel[pktid:" << packet_identifier_ << "]";

                return output.str( );
            }

           private:
            const uint16_t packet_identifier_;
        };  // struct pubrel

    }  // namespace protocol
}  // namespace io_wally
