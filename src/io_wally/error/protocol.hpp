
#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <stdexcept>
#include <memory>

#include <boost/optional.hpp>

#include "io_wally/protocol/common.hpp"

namespace io_wally
{
    /// \brief Custom exceptions.
    ///
    namespace error
    {
        /// \brief Signals an incorrectly encoded MQTT control packet.
        ///
        /// Examples of when a \c malformed_mqtt_packet will be thrown include, but are not limited to:
        ///
        ///  - Incorrectly encoded remaining length
        ///  - Packet is actually shorter than advertised in its remaining length field
        ///  - Username field was flagged as being present in a \c connect packet's variable header, yet is
        ///    actually missing in the payload
        ///
        class malformed_mqtt_packet : public std::runtime_error
        {
           public:
            /// Create a new \c malformed_mqtt_packet instance, using the supplied reason.
            ///
            /// \param what      Reason
            malformed_mqtt_packet( const std::string& what ) : std::runtime_error( what )
            {
                return;
            }
        };
    }
}
