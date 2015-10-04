#pragma once

#include "io_wally/codec/encoder.hpp"
#include "io_wally/protocol/pingresp_packet.hpp"

namespace io_wally
{
    using namespace std;

    namespace encoder
    {
        /// \brief Encoder for PINGRESP packet bodies.
        ///
        template <typename OutputIterator>
        class pingresp_packet_encoder : public packet_body_encoder<OutputIterator>
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
            /// \throws std::invalid_argument   If \c pingresp_packet is not a \c pingresp instance
            OutputIterator encode( const protocol::mqtt_packet& pingresp_packet, OutputIterator buf_start ) const
            {
                if ( pingresp_packet.header( ).type( ) != protocol::packet::Type::PINGRESP )
                    throw invalid_argument( "Supplied packet is not a PINGRESP packet" );

                return buf_start;
            }
        };
    }  /// namespace decoder
}  /// namespace io_wally
