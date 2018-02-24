#pragma once

#include <sstream>
#include <string>

#include "io_wally/protocol/common.hpp"

namespace io_wally
{
    namespace protocol
    {
        /// \brief PINGRESP packet, sent by server in response to \c pingreq packet.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718086
        struct pingresp final : public mqtt_packet
        {
           public:
            /// \brief Create a new \c pingresp instance.
            pingresp( ) : mqtt_packet{0x0D << 4, 0x00}
            {
                assert( packet::type_of( type_and_flags_ ) == packet::Type::PINGRESP );
            }

            /// \return A string representation to be used in log output
            virtual const std::string to_string( ) const override
            {
                std::ostringstream output;
                output << "pingresp[]";

                return output.str( );
            }
        };
    }  // namespace protocol
}  // namespace io_wally
