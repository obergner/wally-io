#pragma once

#include <cassert>

#include "io_wally/codec/encoder.hpp"
#include "io_wally/codec/connack_packet_encoder.hpp"
#include "io_wally/codec/pingresp_packet_encoder.hpp"
#include "io_wally/codec/suback_packet_encoder.hpp"

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
            /// If buffer is too small to accommodate \c mqtt_packet, throw invalid_argument. If \c mqtt_packet
            /// does not conform to spec, throw error::invalid_mqtt_packet.
            ///
            /// \param mqtt_packet      \c mqtt_packet to encode
            /// \param buf_start        Start of buffer to encode \c mqtt_packet into
            /// \param buf_end          End of buffer to encode \c mqtt_packet into
            /// \return         \c OutputIterator that points immediately past the last byte written
            OutputIterator encode( const protocol::mqtt_packet& mqtt_packet,
                                   OutputIterator buf_start,
                                   OutputIterator buf_end ) const
            {
                buf_start = encode_header( mqtt_packet.header( ), buf_start );
                assert( ( buf_end - buf_start ) >= mqtt_packet.header( ).remaining_length( ) );

                return body_encoder_for( mqtt_packet ).encode( mqtt_packet, buf_start );
            }

           private:
            /// Methods
            const packet_body_encoder<OutputIterator>& body_encoder_for(
                const protocol::mqtt_packet& mqtt_packet ) const
            {
                switch ( mqtt_packet.header( ).type( ) )
                {
                    case protocol::packet::Type::CONNACK:
                        return connack_encoder_;
                    case protocol::packet::Type::PINGRESP:
                        return pingresp_encoder_;
                    case protocol::packet::Type::UNSUBACK:
                        return suback_encoder_;
                    case protocol::packet::Type::CONNECT:
                    case protocol::packet::Type::PINGREQ:
                    case protocol::packet::Type::DISCONNECT:
                    case protocol::packet::Type::PUBLISH:
                    case protocol::packet::Type::PUBACK:
                    case protocol::packet::Type::PUBREL:
                    case protocol::packet::Type::PUBREC:
                    case protocol::packet::Type::PUBCOMP:
                    case protocol::packet::Type::SUBSCRIBE:
                    case protocol::packet::Type::SUBACK:
                    case protocol::packet::Type::UNSUBSCRIBE:
                    case protocol::packet::Type::RESERVED1:
                    case protocol::packet::Type::RESERVED2:
                    default:
                        assert( false );
                }
            }

           private:
            /// Fields
            const connack_packet_encoder<OutputIterator> connack_encoder_{};
            const pingresp_packet_encoder<OutputIterator> pingresp_encoder_{};
            const suback_packet_encoder<OutputIterator> suback_encoder_{};
        };

    }  /// namespace decoder
}  /// namespace io_wally
