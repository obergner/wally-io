#pragma once

#include <string>
#include <sstream>

#include "io_wally/protocol/common.hpp"

using boost::uint8_t;

namespace io_wally
{
    namespace protocol
    {
        /// \brief PINGREQ packet, sent by clients to ascertain connectivity.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718081
        struct pingreq : public mqtt_packet
        {
           public:
            /// \brief Create a new \c pingreq instance.
            pingreq( ) : mqtt_packet( packet::header( 0x0C << 4, 0x00 ) )
            {
                return;
            }

            /// \return A string representation to be used in log output
            virtual const std::string to_string( ) const override
            {
                std::ostringstream output;
                output << "pingreq[]";

                return output.str( );
            }
        };
    }  // namespace protocol
}  // namespace io_wally
