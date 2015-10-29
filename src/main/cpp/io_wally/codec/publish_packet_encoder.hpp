#pragma once

#include <cassert>
#include <vector>
#include <algorithm>

#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/codec/encoder.hpp"

namespace io_wally
{
    namespace encoder
    {
        /// \brief Encoder for PUBLISH packet bodies.
        ///
        template <typename OutputIterator>
        class publish_packet_encoder final : public packet_body_encoder<OutputIterator>
        {
           public:
            /// Methods

            /// \brief Encode \c mqtt_packet into supplied buffer.
            ///
            /// Encode \c publish into buffer starting at \c buf_start. Return an \c OutputIterator that points
            /// immediately past the last byte written. If \c publish does not conform to spec, throw
            /// error::invalid_mqtt_packet.
            ///
            /// \param publish_packet \c publish packet to encode
            /// \param buf_start      Start of buffer to encode \c mqtt_packet into
            /// \return         \c OutputIterator that points immediately past the last byte written
            OutputIterator encode( const protocol::mqtt_packet& publish_packet, OutputIterator buf_start ) const
            {
                using namespace io_wally::protocol;

                assert( publish_packet.header( ).type( ) == packet::Type::PUBLISH );

                const publish& publish = dynamic_cast<const struct publish&>( publish_packet );

                // Encode topic
                buf_start = encode_utf8_string( publish.topic( ), buf_start );

                // Encode packet identifier
                buf_start = encode_uint16( publish.packet_identifier( ), buf_start );

                // Encode application message
                std::for_each( publish.application_message( ).begin( ),
                               publish.application_message( ).end( ),
                               [&buf_start]( const uint8_t byte )
                               {
                    *( buf_start++ ) = byte;
                } );

                return buf_start;
            }
        };

    }  /// namespace decoder
}  // namespace io_wally
