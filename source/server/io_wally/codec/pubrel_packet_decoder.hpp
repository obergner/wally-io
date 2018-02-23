#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <tuple>

#include "io_wally/codec/decoder.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/pubrel_packet.hpp"

namespace io_wally
{
    namespace decoder
    {
        /// \brief \c packet_body_decoder implementation for PUBREL packets.
        ///
        /// Interprets supplied \c frame to contain a serialized PUBREL packet. Decodes \c frame and returns
        /// decoded \c pubrel_packet.
        class pubrel_packet_decoder_impl final : public packet_decoder_impl
        {
           public:
            /// \brief Decode supplied \c frame into a \c pubrel packet.
            ///
            /// \see io_wally::protocol::decoder::packet_body_decoder::parse
            ///
            virtual std::shared_ptr<protocol::mqtt_packet> decode( const frame& frame ) const
            {
                using namespace io_wally::protocol;

                assert( frame.type( ) == packet::Type::PUBREL );

                // [MQTT-2.2.2-1] Flags in PUBREL MUST be 0010
                if ( ( frame.type_and_flags & 0x0F ) != 0x02 )
                    throw error::malformed_mqtt_packet{"[MQTT-2.2.2-1] Invalid flags in PUBREL packet."};

                // Parse variable header pubrel_header
                auto packet_id = uint16_t{0};
                std::tie( std::ignore, packet_id ) = decode_uint16( frame.begin, frame.end );

                return std::make_shared<protocol::pubrel>( packet_id );
            }
        };  // class pubrel_packet_decoder_impl
    }       // namespace decoder
}  // namespace io_wally
