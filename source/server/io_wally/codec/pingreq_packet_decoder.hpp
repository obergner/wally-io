#pragma once

#include <cassert>
#include <memory>

#include "io_wally/codec/decoder.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/pingreq_packet.hpp"

namespace io_wally
{
    namespace decoder
    {
        /// \brief \c packet_body_decoder implementation for PINGREQ packets.
        ///
        /// Interprets the supplied buffer to contain a serialized PINGREQ packet. Decodes the buffer and returns
        /// decoded \c pingreq_packet.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718081
        class pingreq_packet_decoder_impl final : public packet_decoder_impl
        {
           public:
            /// \brief Decode the supplied buffer into a \c pingreq_packet.
            ///
            /// \see io_wally::protocol::decoder::packet_body_decoder::decode
            ///
            virtual std::shared_ptr<protocol::mqtt_packet> decode( const frame& frame ) const
            {
                assert( frame.type( ) == protocol::packet::Type::PINGREQ );

                // Check that remaining length is 0, as required by MQTT 3.1.1
                if ( frame.remaining_length( ) != 0 )
                {
                    throw error::malformed_mqtt_packet(
                        "PINGREQ fixed header reports remaining length != 0 (violates MQTT 3.1.1 spec)" );
                }

                return std::make_shared<protocol::pingreq>( );
            }
        };
    }  // namespace decoder
}  // namespace io_wally
