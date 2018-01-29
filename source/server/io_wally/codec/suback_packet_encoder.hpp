#pragma once

#include <cassert>

#include "io_wally/codec/encoder.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/suback_packet.hpp"

namespace io_wally
{
    namespace encoder
    {
        /// \brief Encoder for SUBACK packet bodies.
        ///
        template <typename OutputIterator>
        class suback_packet_encoder final : public packet_body_encoder<OutputIterator>
        {
           public:
            /// Methods

            /// \brief Encode \c mqtt_packet into supplied buffer.
            ///
            /// Encode \c suback into buffer starting at \c buf_start. Return an \c OutputIterator that points
            /// immediately past the last byte written. If \c suback does not conform to spec, throw
            /// error::invalid_mqtt_packet.
            ///
            /// \param suback_packet \c suback packet to encode
            /// \param buf_start      Start of buffer to encode \c mqtt_packet into
            /// \return         \c OutputIterator that points immediately past the last byte written
            OutputIterator encode( const protocol::mqtt_packet& suback_packet, OutputIterator buf_start ) const
            {
                using namespace io_wally::protocol;

                assert( suback_packet.header( ).type( ) == packet::Type::SUBACK );

                const suback& suback = dynamic_cast<const struct suback&>( suback_packet );

                // Encode packet identifier
                buf_start = encode_uint16( suback.packet_identifier( ), buf_start );

                // Encode list of return codes
                for ( auto& rc : suback.return_codes( ) )
                {
                    assert( rc != suback_return_code::RESERVED );
                    *buf_start++ = static_cast<uint8_t>( rc );
                }

                return buf_start;
            }
        };

    }  /// namespace decoder
}  /// namespace io_wally
