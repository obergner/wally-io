#pragma once

#include <memory>

#include "io_wally/protocol/protocol.hpp"

using namespace io_wally;
using namespace io_wally::protocol;

namespace io_wally
{
    // Forward declare mqtt_session(_id) since we cannot have cylic #includes of header files in C++.
    class mqtt_session;
    struct mqtt_session_id;

    /// \brief Namespace defining interfaces to be implemented by "user" code that plugs in to the WallyIO
    ///        framework.
    namespace spi
    {
        class mqtt_authentication_handler
        {
           public:
            virtual const struct connack operator( )( const struct connect& connect ) = 0;
        };

        class mqtt_packet_handler
        {
           public:
            virtual std::unique_ptr<const struct mqtt_ack> operator( )( const struct mqtt_packet& packet ) = 0;
        };  // mqtt_packet_handler

        class mqtt_packet_handler_factory
        {
           public:
            virtual std::unique_ptr<mqtt_authentication_handler> create_authentication_handler(
                const mqtt_session& session ) = 0;

            virtual std::unique_ptr<mqtt_packet_handler> create_packet_handler( const mqtt_session_id& session ) = 0;
        };
    }  // namespace spi
}  // namespace io_wally
