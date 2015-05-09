#pragma once

#include <stdio.h>
#include <string.h>

#include <stdexcept>
#include <memory>
#include <tuple>

#include <boost/optional.hpp>

#include "io_wally/protocol/common.hpp"

using namespace io_wally::protocol;

namespace io_wally
{
    /// \brief Namespace for grouping types and functions related to encoding \c mqtt_packets.
    ///
    /// Most important classes in this namespace are arguably
    ///
    ///  - \c header_encoder and
    ///  - \c mqtt_packet_encoder
    ///
    /// \note       The grunt work of encoding an \c mqtt_packet's on the wire representation is done by several
    ///             implementations of \c packet_body_encoder, one for each concrete \c mqtt_packet.
    namespace encoder
    {

        /// \brief Namespace for grouping all exceptions that may be thrown when encoding an \c mqtt_packet.
        namespace error
        {
            /// \brief Signals an \c mqtt_packet that violates the spec.
            ///
            /// Examples of when a \c malformed_mqtt_packet will be thrown include, but are not limited to:
            ///
            ///  - Packet is longer than allowed by the rules around remaining lengt
            ///
            class illegal_mqtt_packet : public std::runtime_error
            {
               public:
                illegal_mqtt_packet( const std::string& what ) : runtime_error( what )
                {
                    return;
                }
            };
        }  /// namespace error

        /// \brief Encode the \c remaining lenght field into an MQTT fixed header.
        ///
        template <typename OutputIterator>
        class remaining_length_encoder
        {
           public:
            /// \brief Encode supplied \c remaing_length according to the rules mandated by MQTT 3.1.1.
            ///
            /// Encode supplied \c remaining_length assumed to represent an MQTT packet's remaining length (length
            /// sans fixed header) into a buffer starting at \c buf_start. Return an iterator that points
            /// immediately past the last byte written. Throw an
            /// io_wally::encoder::error::illegal_mqtt_packet if \c remaining_length is greater than the
            /// maximum value allowed by MQTT 3.1.1, which is \c 268,435,455.
            ///
            /// Encoding will follow the rules as mandated by MQTT 3.1.1, i.e. using a base of \c 128 (0x80) with
            /// a "carry over" bit.
            ///
            /// \param remaining_length     The remaining length to encode
            /// \param buf_start            Buffer iterator pointing to where to start encdoding
            /// \return                     An iterator pointing immediately past the last byte written into the
            ///                             supplied buffer
            /// \throws io_wally::encoder::error::illegal_mqtt_packet If \c remaining_length is greater
            ///             than maximum value allowed by MQTT 3.1.1: 268,435,455
            ///
            /// \pre        \c buf_start points to second byte in a fixed header
            /// \post       OutputIterator returned points immediately past the last byte written
            ///
            /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718023
            OutputIterator encode( uint32_t remaining_length, OutputIterator buf_start )
            {
                const OutputIterator saved_buf_start = buf_start;
                do
                {
                    uint8_t rest = remaining_length % 0x80;
                    remaining_length = remaining_length / 0x80;
                    uint8_t current = ( remaining_length > 0 ? ( rest | 0x80 ) : rest );
                    *buf_start++ = current;
                } while ( remaining_length > 0 );

                if ( buf_start - saved_buf_start > 4 )
                    throw error::illegal_mqtt_packet( "Supplied remaining_length greater than allowed maximum" );

                return buf_start;
            }
        };

        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718016
        template <typename OutputIterator>
        inline OutputIterator encode_uint16( const uint16_t value, OutputIterator buf_start )
        {
            return buf_start;
        }

        /// \brief Encode a UTF-8 string into the supplied buffer.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718016
        template <typename OutputIterator>
        inline OutputIterator encode_utf8_string( const std::string& value, OutputIterator buf_start )
        {
            return buf_start;
        }

        /// \brief Encode MQTT \c headers.
        ///
        template <typename OutputIterator>
        class header_encoder
        {
           public:
            OutputIterator encode( const packet::header& header, OutputIterator buf_start )
            {
                return buf_start;
            }
        };  // header_encoder

        template <typename OutputIterator>
        class packet_body_encoder
        {
           public:
            virtual OutputIterator encode( const mqtt_packet& packet, OutputIterator buf_start ) = 0;
        };  // packet_body_encoder

    }  /// namespace encoder
}  /// namespace io_wally
