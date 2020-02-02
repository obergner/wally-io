#pragma once

#include <cassert>
#include <memory>

#include "io_wally/codec/decoder.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/disconnect_packet.hpp"

namespace io_wally::decoder
{
    /// \brief \c packet_body_decoder implementation for DISCONNECT packets.
    ///
    /// Interprets the supplied buffer to contain a serialized DISCONNECT packet. Decodes the buffer and returns
    /// decoded \c disconnect_packet.
    ///
    /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718081
    class disconnect_packet_decoder_impl final : public packet_decoder_impl
    {
       public:
        /// \brief Decode the supplied frame into a \c disconnect_packet.
        ///
        /// \see io_wally::protocol::decoder::packet_body_decoder::decode
        ///
        [[nodiscard]] auto decode( const frame& frame ) const -> std::shared_ptr<protocol::mqtt_packet> override
        {
            using namespace io_wally::protocol;

            assert( frame.type( ) == packet::Type::DISCONNECT );

            // MQTT-3.14.1-1: Header flags MUST be zero
            if ( ( frame.type_and_flags & 0x0F ) != 0 )
            {
                throw error::malformed_mqtt_packet{
                    "[MQTT-3.14.1.-1] DISCONNECT header has non-zero flags set (violates MQTT 3.1.1 spec)"};
            }

            // Check that remaining length is 0, as required by MQTT 3.1.1
            if ( frame.remaining_length( ) != 0 )
            {
                throw error::malformed_mqtt_packet{
                    "DISCONNECT fixed header reports remaining length != 0 (violates MQTT 3.1.1 spec)"};
            }

            return std::make_shared<protocol::disconnect>( );
        }
    };
}  // namespace io_wally::decoder
