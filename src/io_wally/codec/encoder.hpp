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
                /// Create a new illegal_mqtt_packet, using the supplied reason.
                ///
                /// \param what      Reason
                illegal_mqtt_packet( const std::string& what ) : runtime_error( what )
                {
                    return;
                }
            };
        }  /// namespace error

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
        template <typename OutputIterator>
        OutputIterator encode_remaining_length( uint32_t remaining_length, OutputIterator buf_start )
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

        /// \brief Encode a 16 bit wide unsigned integer in big endian byte order.
        ///
        /// Take the supplied \c value, encode it in big endian byte order, and write the two bytes thus obtained into
        /// a buffer starting at \c buf_start. Return an \c OutputIterator that points immediately past the last byte
        /// written.
        ///
        /// \param value        The 16 bit wide unsigned integer to encode
        /// \param buf_start    Start of buffer to encode \c value into
        /// \return             An \c OuptutIterator that points immediately past the last byte written
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718016
        template <typename OutputIterator>
        inline OutputIterator encode_uint16( const uint16_t value, OutputIterator buf_start )
        {
            const uint8_t msb = value / 0x0100;
            const uint8_t lsb = value % 0x0100;

            *buf_start++ = msb;
            *buf_start++ = lsb;

            return buf_start;
        }

        /// \brief Maximum string length in bytes allowed by MQTT 3.1.1 - 65536.
        constexpr const uint16_t MAX_STRING_LENGTH = 0x00FF * 0x0100 + 0xFF;

        /// \brief Encode a UTF-8 string into the supplied buffer.
        ///
        /// Take the supplied \c value and write its contents viewed as a byte array into a buffer starting at
        /// \c buf_start. Prefix the byte array with a two bytes sequence encoding \c value length (in bytes) in big
        /// endian byte order. Return an \c OutputIterator that points immediately past the last byte written. If
        /// \c value is longer than the allowed maximum of 65536 bytes throw an error::illegal_mqtt_packet.
        ///
        /// \param value        The string to encode. Note that MQTT 3.1.1 requires strings to be UNICODE strings
        ///                     encoded in UTF-8, although this function does in fact not care.
        /// \param buf_start    Start of buffer to encode \c value into
        /// \return             An \c OutputIterator pointing immediately past the last byte written
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718016
        template <typename OutputIterator>
        inline OutputIterator encode_utf8_string( const std::string& value, OutputIterator buf_start )
        {
            if ( value.length( ) > MAX_STRING_LENGTH )
                throw error::illegal_mqtt_packet( "Supplied UTF-8 string is longer than allowed maximum" );

            const uint16_t string_length = value.length( );
            buf_start = encode_uint16( string_length, buf_start );
            for ( const char& ch : value )
                *buf_start++ = ch;

            return buf_start;
        }

        /// \brief Encode an MQTT \c header.
        ///
        /// Encode \c header into a buffer starting at \c buf_start. Return an \c OutputIterator pointing immediately
        /// past the last byte written. If \c header is not spec compliant, throw an error::illegal_mqtt_packet.
        ///
        /// \param header       The packet::header to encode
        /// \param buf_start    Start of buffer to encode \c header into
        /// \return             An \c OutputIterator pointing immediately past the last byte written
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718020
        template <typename OutputIterator>
        OutputIterator encode_header( const packet::header& header, OutputIterator buf_start )
        {
            *buf_start++ = header.type_and_flags( );
            buf_start = encode_remaining_length( header.remaining_length( ), buf_start );

            return buf_start;
        }

        /// \brief Encoder for \c mqtt_packet bodies, i.e. \c mqtt_packets sans fixed header.
        template <typename OutputIterator>
        class packet_body_encoder
        {
           public:
            /// \brief Encode \c mqtt_packet's body.
            ///
            /// Encode \c packet's body, skipping its \c packet::header, into a buffer starting at \c buf_start.
            /// Return an \c OutputIterator that points immediately past the last byte written. If \c packet
            /// violates the spec, throw an error::illegal_mqtt_packet.
            ///
            /// \param packet        \c mqtt_packet to encode the body of
            /// \param buf_start     Start of buffer to encode packet body into
            /// \return              An \c OutputIterator pointing immediately past the last byte written
            virtual OutputIterator encode( const mqtt_packet& packet, OutputIterator buf_start ) const = 0;
        };  // packet_body_encoder

    }  /// namespace encoder
}  /// namespace io_wally
