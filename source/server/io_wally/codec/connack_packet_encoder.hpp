#pragma once

#include <cassert>
#include <cstdint>

#include "io_wally/codec/encoder.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/connack_packet.hpp"

namespace io_wally
{
    namespace encoder
    {
        /// \brief Encoder for CONNACK packet bodies.
        ///
        template <typename OutputIterator>
        class connack_packet_encoder final : public packet_body_encoder<OutputIterator>
        {
           public:
            /// Methods

            /// \brief Encode \c mqtt_packet into supplied buffer.
            ///
            /// Encode \c connack into buffer starting at \c buf_start. Return an \c OutputIterator that points
            /// immediately past the last byte written. If \c connack does not conform to spec, throw
            /// error::invalid_mqtt_packet.
            ///
            /// \param connack_packet \c connack packet to encode
            /// \param buf_start      Start of buffer to encode \c mqtt_packet into
            /// \return         \c OutputIterator that points immediately past the last byte written
            OutputIterator encode( const protocol::mqtt_packet& connack_packet, OutputIterator buf_start ) const
            {
                using namespace io_wally::protocol;

                assert( connack_packet.type( ) == packet::Type::CONNACK );

                const auto& connack = dynamic_cast<const struct connack&>( connack_packet );
                const auto first_byte = ( connack.is_session_present( ) ? 0x01 : 0x00 );
                auto second_byte = uint8_t{0x00};
                switch ( connack.return_code( ) )
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
                    case connect_return_code::RESERVED:
                        second_byte = 0x06;
                        break;
                    default:
                        assert( false );
                        break;
                }

                *buf_start++ = first_byte;
                *buf_start++ = second_byte;

                return buf_start;
            }
        };
    }  /// namespace decoder
}  /// namespace io_wally
