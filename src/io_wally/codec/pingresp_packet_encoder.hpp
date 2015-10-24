#pragma once

#include <cassert>

#include "io_wally/codec/encoder.hpp"
#include "io_wally/protocol/pingresp_packet.hpp"

namespace io_wally
{
    namespace encoder
    {
        /// \brief Encoder for PINGRESP packet bodies.
        ///
        template <typename OutputIterator>
        class pingresp_packet_encoder final : public packet_body_encoder<OutputIterator>
        {
           public:
            /// Methods

            /// \brief Encode \c mqtt_packet into supplied buffer.
            ///
            /// Encode \c pingresp into buffer starting at \c buf_start. Return an \c OutputIterator that points
            /// immediately past the last byte written.
            ///
            /// \param pingresp_packet \c pingresp packet to encode
            /// \param buf_start      Start of buffer to encode \c mqtt_packet into
            /// \return         \c OutputIterator that points immediately past the last byte written
            OutputIterator encode( const protocol::mqtt_packet& pingresp_packet, OutputIterator buf_start ) const
            {
                assert( pingresp_packet.header( ).type( ) == protocol::packet::Type::PINGRESP );

                return buf_start;
            }
        };
    }  /// namespace decoder
}  /// namespace io_wally
