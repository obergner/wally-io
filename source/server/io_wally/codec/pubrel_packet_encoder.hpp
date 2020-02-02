#pragma once

#include <cassert>

#include "io_wally/codec/encoder.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/pubrel_packet.hpp"

namespace io_wally::encoder
{
    /// \brief Encoder for PUBREL packet bodies.
    ///
    template <typename OutputIterator>
    class pubrel_packet_encoder final : public packet_body_encoder<OutputIterator>
    {
       public:
        /// Methods

        /// \brief Encode \c mqtt_packet into supplied buffer.
        ///
        /// Encode \c pubrel into buffer starting at \c buf_start. Return an \c OutputIterator that points
        /// immediately past the last byte written. If \c pubrel does not conform to spec, throw
        /// error::invalid_mqtt_packet.
        ///
        /// \param pubrel_packet \c pubrel packet to encode
        /// \param buf_start      Start of buffer to encode \c mqtt_packet into
        /// \return         \c OutputIterator that points immediately past the last byte written
        auto encode( const protocol::mqtt_packet& pubrel_packet, OutputIterator buf_start ) const -> OutputIterator
        {
            using namespace io_wally::protocol;

            assert( pubrel_packet.type( ) == packet::Type::PUBREL );

            const auto& pubrel = dynamic_cast<const struct pubrel&>( pubrel_packet );

            // Encode packet identifier
            buf_start = encode_uint16( pubrel.packet_identifier( ), buf_start );

            return buf_start;
        }
    };
}  // namespace io_wally::encoder
