#pragma once

#include <cassert>

#include "io_wally/codec/encoder.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/puback_packet.hpp"

namespace io_wally
{
    namespace encoder
    {
        /// \brief Encoder for PUBACK packet bodies.
        ///
        template <typename OutputIterator>
        class puback_packet_encoder final : public packet_body_encoder<OutputIterator>
        {
           public:
            /// Methods

            /// \brief Encode \c mqtt_packet into supplied buffer.
            ///
            /// Encode \c puback into buffer starting at \c buf_start. Return an \c OutputIterator that points
            /// immediately past the last byte written. If \c puback does not conform to spec, throw
            /// error::invalid_mqtt_packet.
            ///
            /// \param puback_packet \c puback packet to encode
            /// \param buf_start      Start of buffer to encode \c mqtt_packet into
            /// \return         \c OutputIterator that points immediately past the last byte written
            OutputIterator encode( const protocol::mqtt_packet& puback_packet, OutputIterator buf_start ) const
            {
                using namespace io_wally::protocol;

                assert( puback_packet.type( ) == packet::Type::PUBACK );

                const auto& puback = dynamic_cast<const struct puback&>( puback_packet );

                // Encode packet identifier
                buf_start = encode_uint16( puback.packet_identifier( ), buf_start );

                return buf_start;
            }
        };

    }  /// namespace decoder
}  /// namespace io_wally
