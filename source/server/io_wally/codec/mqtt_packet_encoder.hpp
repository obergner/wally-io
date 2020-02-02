#pragma once

#include <cassert>

#include "io_wally/codec/connack_packet_encoder.hpp"
#include "io_wally/codec/encoder.hpp"
#include "io_wally/codec/pingresp_packet_encoder.hpp"
#include "io_wally/codec/puback_packet_encoder.hpp"
#include "io_wally/codec/pubcomp_packet_encoder.hpp"
#include "io_wally/codec/publish_packet_encoder.hpp"
#include "io_wally/codec/pubrec_packet_encoder.hpp"
#include "io_wally/codec/pubrel_packet_encoder.hpp"
#include "io_wally/codec/suback_packet_encoder.hpp"
#include "io_wally/codec/unsuback_packet_encoder.hpp"

namespace io_wally::encoder
{
    /// @brief Encoder for MQTT packets.
    ///
    template <typename OutputIterator>
    class mqtt_packet_encoder final
    {
       public:
        /** @brief Encode @c mqtt_packet into supplied buffer.
         *
         * Encode @c mqtt_packet into buffer starting at @c buf_start, extending to @c buf_end. Return an
         * @c OutputIterator that points immediately past the last byte written.
         * If buffer is too small to accommodate @c mqtt_packet, throw invalid_argument. If @c mqtt_packet
         * does not conform to spec, throw error::invalid_mqtt_packet.
         *
         * @param mqtt_packet      @c mqtt_packet to encode
         * @param buf_start        Start of buffer to encode @c mqtt_packet into
         * @param buf_end          End of buffer to encode @c mqtt_packet into
         * @return         @c OutputIterator that points immediately past the last byte written
         */
        auto encode( const protocol::mqtt_packet& mqtt_packet, OutputIterator buf_start, OutputIterator buf_end ) const
            -> OutputIterator
        {
            buf_start =
                encode_fixed_header( mqtt_packet.type_and_flags( ), mqtt_packet.remaining_length( ), buf_start );
            assert( ( buf_end - buf_start ) >= mqtt_packet.remaining_length( ) );

            return body_encoder_for( mqtt_packet ).encode( mqtt_packet, buf_start );
        }

       private:
        auto body_encoder_for( const protocol::mqtt_packet& mqtt_packet ) const
            -> const packet_body_encoder<OutputIterator>&
        {
            switch ( mqtt_packet.type( ) )
            {
                case protocol::packet::Type::CONNACK:
                    return connack_encoder_;
                case protocol::packet::Type::PINGRESP:
                    return pingresp_encoder_;
                case protocol::packet::Type::SUBACK:
                    return suback_encoder_;
                case protocol::packet::Type::UNSUBACK:
                    return unsuback_encoder_;
                case protocol::packet::Type::PUBLISH:
                    return publish_encoder_;
                case protocol::packet::Type::PUBACK:
                    return puback_encoder_;
                case protocol::packet::Type::PUBREL:
                    return pubrel_encoder_;
                case protocol::packet::Type::PUBREC:
                    return pubrec_encoder_;
                case protocol::packet::Type::PUBCOMP:
                    return pubcomp_encoder_;
                case protocol::packet::Type::CONNECT:
                case protocol::packet::Type::PINGREQ:
                case protocol::packet::Type::DISCONNECT:
                case protocol::packet::Type::SUBSCRIBE:
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
        const unsuback_packet_encoder<OutputIterator> unsuback_encoder_{};
        const publish_packet_encoder<OutputIterator> publish_encoder_{};
        const puback_packet_encoder<OutputIterator> puback_encoder_{};
        const pubrel_packet_encoder<OutputIterator> pubrel_encoder_{};
        const pubrec_packet_encoder<OutputIterator> pubrec_encoder_{};
        const pubcomp_packet_encoder<OutputIterator> pubcomp_encoder_{};
    };

}  // namespace io_wally::encoder
