#pragma once

#include <cassert>
#include <cstdint>
#include <sstream>
#include <memory>
#include <vector>
#include <tuple>

#include "io_wally/protocol/subscription.hpp"
#include "io_wally/protocol/pubrel_packet.hpp"
#include "io_wally/codec/decoder.hpp"

namespace io_wally
{
    namespace decoder
    {
        /// \brief \c packet_body_decoder implementation for PUBREL packets.
        ///
        /// Interprets the supplied buffer to contain a serialized PUBREL packet. Decodes the buffer and returns
        /// decoded \c pubrel_packet.
        template <typename InputIterator>
        class pubrel_packet_decoder final : public packet_body_decoder<InputIterator>
        {
           public:
            /// \brief Decode the supplied buffer into a \c pubrel packet.
            ///
            /// \see io_wally::protocol::decoder::packet_body_decoder::parse
            ///
            virtual std::shared_ptr<protocol::mqtt_packet> decode( const protocol::packet::header& header,
                                                                   InputIterator buf_start,
                                                                   const InputIterator buf_end ) const
            {
                using namespace io_wally::protocol;

                assert( header.type( ) == packet::Type::PUBREL );

                // Check that size of supplied buffer corresponds to remaining length as advertised in header
                if ( ( buf_end - buf_start ) != header.remaining_length( ) )
                {
                    std::ostringstream message;
                    message << "Size of supplied buffer does not correspond to "
                            << "remaining length as advertised in header: [expected:" << header.remaining_length( )
                            << "|actual:" << ( buf_end - buf_start ) << "]";
                    throw error::malformed_mqtt_packet( message.str( ) );
                }

                // [MQTT-2.2.2-1] Flags in PUBREL MUST be 0010
                if ( ( header.type_and_flags( ) & 0x0F ) != 0x02 )
                    throw error::malformed_mqtt_packet( "[MQTT-2.2.2-1] Invalid flags in PUBREL packet." );

                InputIterator new_buf_start = buf_start;

                // Parse variable header pubrel_header
                uint16_t packet_identifier = -1;
                std::tie( new_buf_start, packet_identifier ) = decode_uint16( new_buf_start, buf_end );

                return std::make_shared<protocol::pubrel>( packet_identifier );
            }
        };  // class pubrel_packet_decoder

    }  // namespace decoder
}  // namespace io_wally
