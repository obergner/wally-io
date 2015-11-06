#pragma once

#include <cassert>
#include <memory>

#include "io_wally/protocol/common.hpp"

#include "io_wally/codec/connect_packet_decoder.hpp"
#include "io_wally/codec/disconnect_packet_decoder.hpp"
#include "io_wally/codec/pingreq_packet_decoder.hpp"
#include "io_wally/codec/subscribe_packet_decoder.hpp"
#include "io_wally/codec/publish_packet_decoder.hpp"
#include "io_wally/codec/puback_packet_decoder.hpp"
#include "io_wally/codec/pubrec_packet_decoder.hpp"
#include "io_wally/codec/pubcomp_packet_decoder.hpp"

namespace io_wally
{
    namespace decoder
    {
        /// \brief Decoder for MQTT packets of arbitrary but known type.
        ///
        /// Takes an MQTT \c header and a buffer containing an MQTT packet's on the wire representation. Deduces
        /// from the supplied \c header the type of \c mqtt_packet to decode and delegates to an appropriate
        /// \c packet_body_decoder implementation.
        template <typename InputIterator>
        class mqtt_packet_decoder final
        {
           public:
            /// Methods

            /// \brief Decode the supplied buffer into an MQTT packet.
            ///
            /// From the supplied \c header determine the concrete type of \c mqtt_packet to decode. Find an
            /// appropriate implementation of \c packet_body_decoder and delegate decoding to it. Return the decoded
            /// \c mqtt_packet, transferring ownership to the caller. If decoding fails throw an
            /// \c error::malformed_mqtt_packet.
            ///
            /// \param header       Parsed \c header of MQTT control packet to decode.
            /// \param buf_start    Start of buffer containing the serialized MQTT packet. MUST point immediately
            ///                     past the fixed header.
            /// \param buf_end      End of buffer containing the serialized MQTT packet.
            /// \return             The decoded \c mqtt_packet, i.e. an instance of a concrete subclass of
            ///                     \c mqtt_packet. Note that the caller assumes ownership.
            /// \throws error::malformed_mqtt_packet    If encoding is malformed, e.g. remaining length has been
            ///                                         incorrectly encoded.
            ///
            /// \pre        \c buf_start points to the first byte after the fixed header in a buffer representing
            ///             an \c mqtt_packet's on the wire format.
            /// \pre        \c buf_end points immediately past the last byte in a buffer representing an
            ///             \c mqtt_packet's on the wire format.
            std::shared_ptr<const protocol::mqtt_packet> decode( const protocol::packet::header& header,
                                                                 InputIterator buf_start,
                                                                 const InputIterator buf_end ) const
            {
                return body_decoder_for( header ).decode( header, buf_start, buf_end );
            }

           private:
            /// Methods
            const packet_body_decoder<InputIterator>& body_decoder_for( const protocol::packet::header& header ) const
            {
                switch ( header.type( ) )
                {
                    case protocol::packet::Type::CONNECT:
                        return connect_packet_decoder_;
                    case protocol::packet::Type::PINGREQ:
                        return pingreq_packet_decoder_;
                    case protocol::packet::Type::DISCONNECT:
                        return disconnect_packet_decoder_;
                    case protocol::packet::Type::SUBSCRIBE:
                        return subscribe_packet_decoder_;
                    case protocol::packet::Type::PUBLISH:
                        return publish_packet_decoder_;
                    case protocol::packet::Type::PUBACK:
                        return puback_packet_decoder_;
                    case protocol::packet::Type::PUBREC:
                        return pubrec_packet_decoder_;
                    case protocol::packet::Type::PUBCOMP:
                        return pubcomp_packet_decoder_;
                    case protocol::packet::Type::CONNACK:
                    case protocol::packet::Type::PUBREL:
                    case protocol::packet::Type::SUBACK:
                    case protocol::packet::Type::UNSUBSCRIBE:
                    case protocol::packet::Type::UNSUBACK:
                    case protocol::packet::Type::PINGRESP:
                    case protocol::packet::Type::RESERVED1:
                    case protocol::packet::Type::RESERVED2:
                    default:
                        throw std::invalid_argument( "Unsupported MQTT control packet type" );
                }

                assert( false );
            }

           private:
            /// Fields
            const connect_packet_decoder<InputIterator> connect_packet_decoder_{};
            const pingreq_packet_decoder<InputIterator> pingreq_packet_decoder_{};
            const disconnect_packet_decoder<InputIterator> disconnect_packet_decoder_{};
            const subscribe_packet_decoder<InputIterator> subscribe_packet_decoder_{};
            const publish_packet_decoder<InputIterator> publish_packet_decoder_{};
            const puback_packet_decoder<InputIterator> puback_packet_decoder_{};
            const pubrec_packet_decoder<InputIterator> pubrec_packet_decoder_{};
            const pubcomp_packet_decoder<InputIterator> pubcomp_packet_decoder_{};
        };

    }  /// namespace decoder
}  /// namespace io_wally
