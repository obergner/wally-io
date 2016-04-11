#pragma once

#include <cassert>

#include "io_wally/codec/encoder.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/unsuback_packet.hpp"

namespace io_wally
{
    namespace encoder
    {
        /// \brief Encoder for UNSUBACK packet bodies.
        ///
        template <typename OutputIterator>
        class unsuback_packet_encoder final : public packet_body_encoder<OutputIterator>
        {
           public:
            /// Methods

            /// \brief Encode \c mqtt_packet into supplied buffer.
            ///
            /// Encode \c unsuback into buffer starting at \c buf_start. Return an \c OutputIterator that points
            /// immediately past the last byte written. If \c unsuback does not conform to spec, throw
            /// error::invalid_mqtt_packet.
            ///
            /// \param unsuback_packet \c unsuback packet to encode
            /// \param buf_start      Start of buffer to encode \c mqtt_packet into
            /// \return         \c OutputIterator that points immediately past the last byte written
            OutputIterator encode( const protocol::mqtt_packet& unsuback_packet, OutputIterator buf_start ) const
            {
                using namespace io_wally::protocol;

                assert( unsuback_packet.header( ).type( ) == packet::Type::UNSUBACK );

                const unsuback& unsuback = dynamic_cast<const struct unsuback&>( unsuback_packet );

                // Encode packet identifier
                buf_start = encode_uint16( unsuback.packet_identifier( ), buf_start );

                return buf_start;
            }
        };

    }  /// namespace decoder
}  /// namespace io_wally
