#pragma once

#include <string>
#include <sstream>

#include "io_wally/protocol/common.hpp"

namespace io_wally
{
    using namespace std;

    namespace protocol
    {
        /// \brief DISCONNECT packet, sent by clients when about to terminate a session.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718090
        struct disconnect : public mqtt_packet
        {
           public:
            /// \brief Create a new \c disconnect instance.
            disconnect( ) : mqtt_packet{packet::header( 0x0E << 4, 0x00 )}
            {
                return;
            }

            /// \return A string representation to be used in log output
            virtual const string to_string( ) const override
            {
                ostringstream output;
                output << "disconnect[]";

                return output.str( );
            }
        };
    }  // namespace protocol
}  // namespace io_wally
