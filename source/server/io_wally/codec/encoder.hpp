#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

#include "io_wally/error/protocol.hpp"
#include "io_wally/protocol/common.hpp"

namespace io_wally::encoder
{
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
    /// \throws io_wally::error::malformed_mqtt_packet If \c remaining_length is greater
    ///             than maximum value allowed by MQTT 3.1.1: 268,435,455
    ///
    /// \pre        \c buf_start points to second byte in a fixed header
    /// \post       OutputIterator returned points immediately past the last byte written
    ///
    /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718023
    template <typename OutputIterator>
    auto encode_remaining_length( uint32_t remaining_length, OutputIterator buf_start ) -> OutputIterator
    {
        const OutputIterator saved_buf_start = buf_start;
        do
        {
            const auto rest = remaining_length % 0x80;
            remaining_length = remaining_length / 0x80;
            const auto current = ( remaining_length > 0 ? ( rest | 0x80 ) : rest );
            *buf_start++ = current;
        } while ( remaining_length > 0 );

        if ( buf_start - saved_buf_start > 4 )
            throw error::malformed_mqtt_packet( "Supplied remaining_length greater than allowed maximum" );

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
    inline auto encode_uint16( const uint16_t value, OutputIterator buf_start ) -> OutputIterator
    {
        const auto msb = value / 0x0100;
        const auto lsb = value % 0x0100;

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
    /// \throws io_wally::error::malformed_mqtt_packet If \c value is greater than maximum value allowed by
    //          MQTT 3.1.1: 65536
    ///
    /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718016
    template <typename OutputIterator>
    inline auto encode_utf8_string( const std::string& value, OutputIterator buf_start ) -> OutputIterator
    {
        if ( value.length( ) > MAX_STRING_LENGTH )
            throw error::malformed_mqtt_packet( "Supplied UTF-8 string is longer than allowed maximum" );

        const auto string_length = value.length( );
        buf_start = encode_uint16( string_length, buf_start );
        for ( const char& ch : value )
            *buf_start++ = ch;

        return buf_start;
    }

    /**
     * @brief Encode an MQTT @c header.
     *
     * Encode @c type_and_flags and @c remaining_length into a buffer starting at @c buf_start. Return an @c
     * OutputIterator pointing immediately past the last byte written. If @c remaining_length is not spec compliant,
     * throw an error::illegal_mqtt_packet.
     *
     * @param type_and_flags    The first header byte containing @c packet::Type and flags
     * @param remaining_length  MQTT packet's remaining length
     * @param buf_start    Start of buffer to encode @c header into
     * @return             An @c OutputIterator pointing immediately past the last byte written
     *
     * @see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718020
     */
    template <typename OutputIterator>
    auto encode_fixed_header( const uint8_t type_and_flags, const uint32_t remaining_length, OutputIterator buf_start )
        -> OutputIterator
    {
        *buf_start++ = type_and_flags;
        buf_start = encode_remaining_length( remaining_length, buf_start );

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
        virtual auto encode( const protocol::mqtt_packet& packet, OutputIterator buf_start ) const
            -> OutputIterator = 0;
    };  // packet_body_encoder

}  // namespace io_wally::encoder
