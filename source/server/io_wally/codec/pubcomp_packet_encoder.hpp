#pragma once

#include <cassert>

#include "io_wally/codec/encoder.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/pubcomp_packet.hpp"

namespace io_wally
{
    namespace encoder
    {
        /// \brief Encoder for PUBCOMP packet bodies.
        ///
        template <typename OutputIterator>
        class pubcomp_packet_encoder final : public packet_body_encoder<OutputIterator>
        {
           public:
            /// Methods

            /// \brief Encode \c mqtt_packet into supplied buffer.
            ///
            /// Encode \c pubcomp into buffer starting at \c buf_start. Return an \c OutputIterator that points
            /// immediately past the last byte written. If \c pubcomp does not conform to spec, throw
            /// error::invalid_mqtt_packet.
            ///
            /// \param pubcomp_packet \c pubcomp packet to encode
            /// \param buf_start      Start of buffer to encode \c mqtt_packet into
            /// \return         \c OutputIterator that points immediately past the last byte written
            OutputIterator encode( const protocol::mqtt_packet& pubcomp_packet, OutputIterator buf_start ) const
            {
                using namespace io_wally::protocol;

                assert( pubcomp_packet.type( ) == packet::Type::PUBCOMP );

                const auto& pubcomp = dynamic_cast<const struct pubcomp&>( pubcomp_packet );

                // Encode packet identifier
                buf_start = encode_uint16( pubcomp.packet_identifier( ), buf_start );

                return buf_start;
            }
        };

    }  /// namespace decoder
}  /// namespace io_wally
