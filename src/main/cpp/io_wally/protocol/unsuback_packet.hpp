#pragma once

#include <cstdint>
#include <sstream>
#include <vector>

#include "io_wally/protocol/common.hpp"

namespace io_wally
{
    namespace protocol
    {
        /// \brief UNSUBACK packet, sent in response to a \c suback packet.
        ///
        /// A UNSUBACK packet contains in its variable header the \c packet_identifier sent by a client in the
        /// corresponding UNSUBSCRIBE packet.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718068
        struct unsuback final : public mqtt_ack
        {
           public:
            unsuback( const uint16_t packet_identifier )
                : mqtt_ack{packet::Type::SUBACK, 2}, packet_identifier_{packet_identifier}
            {
            }

            /// \brief Return \c packet_identifier sent in corresponding UNSUBSCRIBE packet.
            ///
            /// \return \c packet_identifier sent in corresponding UNSUBSCRIBE packet.
            uint16_t packet_identifier( ) const
            {
                return packet_identifier_;
            }

            /// \return A string representation to be used in log output
            virtual const std::string to_string( ) const override
            {
                auto output = std::ostringstream{};
                output << "unsuback[pktid:" << packet_identifier_ << "]";

                return output.str( );
            }

           private:
            const uint16_t packet_identifier_;
        };  // struct suback

    }  // namespace protocol
}  // namespace io_wally
