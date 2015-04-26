#pragma once

#include <boost/cstdint.hpp>

using boost::uint8_t;

namespace io_wally
{
    namespace protocol
    {
        /**
         * Namespace for data types shared across all MQTT packets:
         *
         * - quality of service
         * - control packet type
         * - protocol level
         * - header flags
         * - fixed header
         *
         */
        namespace packet
        {

            const uint8_t CONTROL_PACKET_TYPE_MASK = 0xF0;

            typedef enum : int
            {
                AT_MOST_ONCE,
                AT_LEAST_ONCE,
                EXACTLY_ONCE,
                RESERVED
            } qos;

            typedef enum : int
            {
                RESERVED1 = 0,
                CONNECT = 1,
                CONNACK,
                PUBLISH,
                PUBACK,
                PUBREC,
                PUBREL,
                PUBCOMP,
                SUBSCRIBE,
                SUBACK,
                UNSUBSCRIBE,
                UNSUBACK,
                PINGREQ,
                PINGRESP,
                DISCONNECT,
                RESERVED2
            } type;

            typedef enum : int
            {
                LEVEL4 = 0x04,
                UNSUPPORTED
            } protocol_level;

            struct header_flags
            {
               public:
                header_flags( const uint8_t& flags ) : flags_( flags )
                {
                }

                bool dup( ) const
                {
                    return ( flags_ & 0x08 ) == 0x08;
                }

                bool retain( ) const
                {
                    return ( flags_ & 0x01 ) == 0x01;
                }

                const packet::qos qos( ) const
                {
                    const uint8_t qos_bits = ( flags_ & 0x06 ) >> 1;
                    packet::qos res;
                    switch ( qos_bits )
                    {
                        case 0x00:
                            res = packet::AT_MOST_ONCE;
                            break;
                        case 0x01:
                            res = packet::AT_LEAST_ONCE;
                            break;
                        case 0x02:
                            res = packet::EXACTLY_ONCE;
                            break;
                        default:
                            res = packet::RESERVED;
                            break;
                    }
                    return res;
                }

               private:
                const uint8_t& flags_;
            };

            struct header
            {
               public:
                header( const uint8_t type_and_flags ) : control_packet_type_and_flags_( type_and_flags )
                {
                }

                const packet::type type( ) const
                {
                    const uint8_t type_bits = ( control_packet_type_and_flags_ & CONTROL_PACKET_TYPE_MASK ) >> 4;
                    packet::type res;
                    switch ( type_bits )
                    {
                        case 0x00:
                            res = packet::RESERVED1;
                            break;
                        case 0x01:
                            res = packet::CONNECT;
                            break;
                        case 0x02:
                            res = packet::CONNACK;
                            break;
                        case 0x03:
                            res = packet::PUBLISH;
                            break;
                        case 0x04:
                            res = packet::PUBACK;
                            break;
                        case 0x05:
                            res = packet::PUBREC;
                            break;
                        case 0x06:
                            res = packet::PUBREL;
                            break;
                        case 0x07:
                            res = packet::PUBCOMP;
                            break;
                        case 0x08:
                            res = packet::SUBSCRIBE;
                            break;
                        case 0x09:
                            res = packet::SUBACK;
                            break;
                        case 0x0A:
                            res = packet::UNSUBSCRIBE;
                            break;
                        case 0x0B:
                            res = packet::UNSUBACK;
                            break;
                        case 0x0C:
                            res = packet::PINGREQ;
                            break;
                        case 0x0D:
                            res = packet::PINGRESP;
                            break;
                        case 0x0E:
                            res = packet::DISCONNECT;
                            break;
                        default:
                            res = packet::RESERVED2;
                            break;
                    }
                    return res;
                }

                const packet::header_flags flags( ) const
                {
                    return packet::header_flags( control_packet_type_and_flags_ );
                }

               private:
                const uint8_t control_packet_type_and_flags_;
            };

            class remaining_length
            {
               public:
                typedef enum : int
                {
                    INCOMPLETE = 0,
                    COMPLETE,
                    OUT_OF_RANGE
                } parse_state;

                remaining_length( )
                {
                }

                parse_state operator( )( uint32_t& result, const uint8_t next_byte )
                {
                    current_ += ( next_byte & ~MSB_MASK ) * multiplier_;
                    parse_state pst = ( next_byte & MSB_MASK ) == MSB_MASK ? INCOMPLETE : COMPLETE;
                    if ( pst == INCOMPLETE )
                        multiplier_ *= 128;

                    if ( multiplier_ > MAX_MULTIPLIER )
                        pst = OUT_OF_RANGE;
                    else if ( pst == COMPLETE )
                        result = current_;

                    return pst;
                }

               private:
                const uint8_t MSB_MASK = 0x80;
                const uint32_t MAX_MULTIPLIER = 128 * 128 * 128;
                uint32_t current_ = 0;
                uint32_t multiplier_ = 1;
            };
        }  // namespace packet
    }      // namespace protocol
}  // namespace io_wally
