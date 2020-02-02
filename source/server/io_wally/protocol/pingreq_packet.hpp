#pragma once

#include <sstream>
#include <string>

#include "io_wally/protocol/common.hpp"

namespace io_wally::protocol
{
    /// \brief PINGREQ packet, sent by clients to ascertain connectivity.
    ///
    /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718081
    struct pingreq final : public mqtt_packet
    {
       public:
        /// \brief Create a new \c pingreq instance.
        pingreq( ) : mqtt_packet{0x0C << 4, 0x00}
        {
            assert( packet::type_of( type_and_flags_ ) == packet::Type::PINGREQ );
        }

        /// \return A string representation to be used in log output
        [[nodiscard]] auto to_string( ) const -> const std::string override
        {
            std::ostringstream output;
            output << "pingreq[]";

            return output.str( );
        }
    };
}  // namespace io_wally::protocol
