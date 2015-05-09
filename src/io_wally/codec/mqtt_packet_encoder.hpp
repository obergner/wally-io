#pragma once

#include "io_wally/codec/encoder.hpp"

using namespace io_wally::protocol;

namespace io_wally
{
    namespace encoder
    {
        /// \brief Encoder for MQTT packets.
        ///
        template <typename OutputIterator>
        class mqtt_packet_encoder
        {
           public:
            /// Methods

            /// \brief Encode \c mqtt_packet into supplied buffer.
            ///
            /// Encode \c mqtt_packet into buffer starting at \c buf_start, extending to \c buf_end. Return an
            /// \c OutputIterator that points immediately past the last byte written.
            /// If buffer is too small to accommodate \c mqtt_packet, throw std::invalid_argument. If \c mqtt_packet
            /// does not conform to spec, throw error::invalid_mqtt_packet.
            ///
            /// \param mqtt_packet      \c mqtt_packet to encode
            /// \param buf_start        Start of buffer to encode \c mqtt_packet into
            /// \param buf_end          End of buffer to encode \c mqtt_packet into
            /// \return         \c OutputIterator that points immediately past the last byte written
            /// \throws std::invalid_argument   If buffer is too small for \c mqtt_packet
            /// \throws error::invalid_mqtt_packet      If \c mqtt_packet does not conform to spec
            OutputIterator encode( const mqtt_packet& mqtt_packet, OutputIterator buf_start, OutputIterator buf_end )
            {
                buf_start = encode_header( mqtt_packet.header( ), buf_start );
                if ( ( buf_end - buf_start ) < mqtt_packet.header( ).remaining_length( ) )
                    throw std::invalid_argument( "Supplied buffer is too small for encoding mqtt packet" );

                return body_encoder_for( mqtt_packet ).encode( mqtt_packet, buf_start );
            }

           private:
            /// Methods
            const packet_body_encoder<OutputIterator>& body_encoder_for( const mqtt_packet& mqtt_packet )
            {
                assert( false );
            }
        };

    }  /// namespace decoder
}  /// namespace io_wally
