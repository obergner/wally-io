#pragma once

#include <iostream>
#include <stdexcept>

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
                header_flags( const uint8_t flags ) : flags_( flags )
                {
                    return;
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
                const uint8_t flags_;
            };  /// struct header_flags

            inline std::ostream& operator<<( std::ostream& output, header_flags const& header_flags )
            {
                std::string qos_string;
                switch ( header_flags.qos( ) )
                {
                    case AT_MOST_ONCE:
                        qos_string = "At most once";
                        break;
                    case AT_LEAST_ONCE:
                        qos_string = "At least once";
                        break;
                    case EXACTLY_ONCE:
                        qos_string = "Exactly once";
                        break;
                    default:
                        qos_string = "Reserved";
                        break;
                }

                output << "header_flags[dup:" << header_flags.dup( ) << "|retain:" << header_flags.retain( )
                       << "|qos:" << qos_string << "]";

                return output;
            }

            struct header
            {
               public:
                header( const uint8_t type_and_flags, const uint32_t remaining_length )
                    : control_packet_type_and_flags_( type_and_flags ), remaining_length_( remaining_length )
                {
                    return;
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

                const uint32_t remaining_length( ) const
                {
                    return remaining_length_;
                }

               private:
                const uint8_t control_packet_type_and_flags_;
                const uint32_t remaining_length_;
            };  /// struct header

            inline std::ostream& operator<<( std::ostream& output, header const& header )
            {
                std::string type_string;
                switch ( header.type( ) )
                {
                    case packet::RESERVED1:
                        type_string = "Reserved1";
                        break;
                    case packet::CONNECT:
                        type_string = "Connect";
                        break;
                    case packet::CONNACK:
                        type_string = "Connack";
                        break;
                    case packet::PUBLISH:
                        type_string = "Publish";
                        break;
                    case packet::PUBACK:
                        type_string = "Puback";
                        break;
                    case packet::PUBREC:
                        type_string = "Pubrec";
                        break;
                    case packet::PUBCOMP:
                        type_string = "Pubcomp";
                        break;
                    case packet::SUBSCRIBE:
                        type_string = "Subscribe";
                        break;
                    case packet::SUBACK:
                        type_string = "Suback";
                        break;
                    case packet::UNSUBSCRIBE:
                        type_string = "Unsubscribe";
                        break;
                    case packet::UNSUBACK:
                        type_string = "Unsuback";
                        break;
                    case packet::PINGREQ:
                        type_string = "Pingreq";
                        break;
                    case packet::PINGRESP:
                        type_string = "Pingresp";
                        break;
                    case packet::DISCONNECT:
                        type_string = "Disconnect";
                        break;
                    default:
                        type_string = "Reserved2";
                        break;
                }

                output << "header[type:" << type_string << "|" << header.flags( ) << "]";

                return output;
            }

            /// \brief Stateful functor for parsing the 'remaining lenght' field in an MQTT fixed header.
            ///
            class remaining_length
            {
               public:
                typedef enum : int
                {
                    INCOMPLETE = 0,
                    COMPLETE
                } parse_state;

                remaining_length( )
                {
                    return;
                }

                /// \brief Calculate an MQTT packet's 'remaining length', i.e. its length in bytes minus fixed header
                ///        length.
                ///
                /// Take the next length byte 'next_byte'. Return 'parse_state' INCOMPLETE while calculation is not
                /// yet done. Once calculation has completed, return 'parse_state' COMPLETE and assign calculated
                /// 'remaining_lenght' to out parameter 'result'.
                ///
                /// Throw std::range_error if provided sequence of bytes does not encode a valid 'remaining length'.
                ///
                /// \param result Remaining length calculated by this functor.
                /// \param next_byte Next byte in sequence of bytes encoding an MQTT packet's remaining length.
                /// \return Current parse state, either INCOMPLETE or COMPLETE.
                /// \throws std::range_error If provided sequence of bytes does not encode a valid 'remaining length'.
                parse_state operator( )( uint32_t& result, const uint8_t next_byte )
                {
                    current_ += ( next_byte & ~MSB_MASK ) * multiplier_;
                    parse_state pst = ( next_byte & MSB_MASK ) == MSB_MASK ? INCOMPLETE : COMPLETE;
                    if ( pst == INCOMPLETE )
                        multiplier_ *= 128;

                    if ( multiplier_ > MAX_MULTIPLIER )
                        throw std::range_error( "supplied byte sequence does not encode a valid remaining length" );
                    if ( pst == COMPLETE )
                        result = current_;

                    return pst;
                }

                /// \brief Reset this functor's internal state so that it may be reused.
                void reset( )
                {
                    current_ = 0;
                    multiplier_ = 1;
                }

               private:
                const uint8_t MSB_MASK = 0x80;
                const uint32_t MAX_MULTIPLIER = 128 * 128 * 128;
                uint32_t current_ = 0;
                uint32_t multiplier_ = 1;
            };
        }  // namespace packet

        struct mqtt_packet
        {
           public:
            mqtt_packet( struct packet::header header ) : header_( std::move( header ) )
            {
                return;
            }

            const struct packet::header& header( ) const
            {
                return header_;
            }

           private:
            const struct packet::header header_;
        };

        inline std::ostream& operator<<( std::ostream& output, mqtt_packet const& mqtt_packet )
        {
            output << "mqtt_packet[header:" << mqtt_packet.header( ) << "]";

            return output;
        }

    }  // namespace protocol
}  // namespace io_wally
