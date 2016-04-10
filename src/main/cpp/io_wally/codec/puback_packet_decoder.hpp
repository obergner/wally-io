#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <tuple>

#include "io_wally/protocol/puback_packet.hpp"
#include "io_wally/codec/decoder.hpp"

namespace io_wally
{
    namespace decoder
    {
        /// \brief \c packet_body_decoder implementation for PUBACK packets.
        ///
        /// Interprets the supplied frame to contain a serialized PUBACK packet. Decodes the buffer and returns
        /// decoded \c puback_packet.
        class puback_packet_decoder_impl final : public packet_decoder_impl
        {
           public:
            /// \brief Decode the supplied buffer into a \c puback packet.
            ///
            /// \see io_wally::protocol::decoder::packet_body_decoder::parse
            ///
            virtual std::shared_ptr<protocol::mqtt_packet> decode( const frame& frame ) const
            {
                using namespace io_wally::protocol;

                assert( frame.type( ) == packet::Type::PUBACK );

                // [MQTT-2.2.2-1] Flags in PUBACK MUST be zero
                if ( ( frame.type_and_flags & 0x0F ) != 0x00 )
                    throw error::malformed_mqtt_packet( "[MQTT-2.2.2-1] Invalid flags in PUBACK packet." );

                // Parse variable header puback_header
                auto packet_id = uint16_t{0};
                std::tie( std::ignore, packet_id ) = decode_uint16( frame.begin, frame.end );

                return std::make_shared<protocol::puback>( packet_id );
            }
        };  // class puback_packet_decoder_impl
    }       // namespace decoder
}  // namespace io_wally
