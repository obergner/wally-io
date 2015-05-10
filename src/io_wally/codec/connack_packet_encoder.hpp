#pragma once

#include "io_wally/codec/encoder.hpp"
#include "io_wally/protocol/connack_packet.hpp"

using namespace io_wally::protocol;

namespace io_wally
{
    namespace encoder
    {
        /// \brief Encoder for CONNACK packet bodies.
        ///
        template <typename OutputIterator>
        class connack_packet_encoder : public packet_body_encoder<OutputIterator, connack>
        {
           public:
            /// Methods

            /// \brief Encode \c mqtt_packet into supplied buffer.
            ///
            /// Encode \c connack into buffer starting at \c buf_start. Return an \c OutputIterator that points
            /// immediately past the last byte written. If \c connack does not conform to spec, throw
            /// error::invalid_mqtt_packet.
            ///
            /// \param connack        \c connack packet to encode
            /// \param buf_start      Start of buffer to encode \c mqtt_packet into
            /// \return         \c OutputIterator that points immediately past the last byte written
            /// \throws std::invalid_argument   If buffer is too small for \c connack
            /// \throws error::invalid_mqtt_packet      If \c connack does not conform to spec
            OutputIterator encode( const connack& connack, OutputIterator buf_start )
            {
                const uint8_t first_byte = ( connack.connack_header( ).is_session_present( ) ? 0x01 : 0x00 );
                uint8_t second_byte;
                switch ( connack.connack_header( ).return_code( ) )
                {
                    case connect_return_code::CONNECTION_ACCEPTED:
                        second_byte = 0x00;
                        break;
                    case connect_return_code::UNACCEPTABLE_PROTOCOL_VERSION:
                        second_byte = 0x01;
                        break;
                    case connect_return_code::IDENTIFIER_REJECTED:
                        second_byte = 0x02;
                        break;
                    case connect_return_code::SERVER_UNAVAILABLE:
                        second_byte = 0x03;
                        break;
                    case connect_return_code::BAD_USERNAME_OR_PASSWORD:
                        second_byte = 0x04;
                        break;
                    case connect_return_code::NOT_AUTHORIZED:
                        second_byte = 0x05;
                        break;
                    default:
                        second_byte = 0x06;
                        break;
                }

                *buf_start++ = first_byte;
                *buf_start++ = second_byte;

                return buf_start;
            }
        };

    }  /// namespace decoder
}  /// namespace io_wally
