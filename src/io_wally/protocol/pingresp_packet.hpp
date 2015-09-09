#pragma once

#include <string>
#include <sstream>

#include <boost/cstdint.hpp>
#include <boost/optional.hpp>

#include "io_wally/protocol/common.hpp"

using boost::uint8_t;

namespace io_wally
{
    namespace protocol
    {
        /// \brief PINGRESP packet, sent by server in response to \c pingreq packet.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718086
        struct pingresp : public mqtt_packet
        {
           public:
            /// \brief Create a new \c pingresp instance.
            pingresp( ) : mqtt_packet( packet::header( 0x0D << 4, 0x00 ) )
            {
                return;
            }

            /// \return A string representation to be used in log output
            virtual const std::string to_string( ) const override
            {
                std::ostringstream output;
                output << "pingresp[" << header( ) << "]";

                return output.str( );
            }
        };
    }  // namespace protocol
}  // namespace io_wally
