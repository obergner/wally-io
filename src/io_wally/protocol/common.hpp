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

            enum class QoS : int
            {
                AT_MOST_ONCE,
                AT_LEAST_ONCE,
                EXACTLY_ONCE,
                RESERVED
            };

            enum class Type : int
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
            };

            enum class ProtocolLevel : int
            {
                LEVEL4 = 0x04,
                UNSUPPORTED
            };

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

                const packet::QoS qos( ) const
                {
                    const uint8_t qos_bits = ( flags_ & 0x06 ) >> 1;
                    packet::QoS res;
                    switch ( qos_bits )
                    {
                        case 0x00:
                            res = packet::QoS::AT_MOST_ONCE;
                            break;
                        case 0x01:
                            res = packet::QoS::AT_LEAST_ONCE;
                            break;
                        case 0x02:
                            res = packet::QoS::EXACTLY_ONCE;
                            break;
                        default:
                            res = packet::QoS::RESERVED;
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
                    case packet::QoS::AT_MOST_ONCE:
                        qos_string = "At most once";
                        break;
                    case packet::QoS::AT_LEAST_ONCE:
                        qos_string = "At least once";
                        break;
                    case packet::QoS::EXACTLY_ONCE:
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

                const packet::Type type( ) const
                {
                    const uint8_t type_bits = ( control_packet_type_and_flags_ & CONTROL_PACKET_TYPE_MASK ) >> 4;
                    packet::Type res;
                    switch ( type_bits )
                    {
                        case 0x00:
                            res = packet::Type::RESERVED1;
                            break;
                        case 0x01:
                            res = packet::Type::CONNECT;
                            break;
                        case 0x02:
                            res = packet::Type::CONNACK;
                            break;
                        case 0x03:
                            res = packet::Type::PUBLISH;
                            break;
                        case 0x04:
                            res = packet::Type::PUBACK;
                            break;
                        case 0x05:
                            res = packet::Type::PUBREC;
                            break;
                        case 0x06:
                            res = packet::Type::PUBREL;
                            break;
                        case 0x07:
                            res = packet::Type::PUBCOMP;
                            break;
                        case 0x08:
                            res = packet::Type::SUBSCRIBE;
                            break;
                        case 0x09:
                            res = packet::Type::SUBACK;
                            break;
                        case 0x0A:
                            res = packet::Type::UNSUBSCRIBE;
                            break;
                        case 0x0B:
                            res = packet::Type::UNSUBACK;
                            break;
                        case 0x0C:
                            res = packet::Type::PINGREQ;
                            break;
                        case 0x0D:
                            res = packet::Type::PINGRESP;
                            break;
                        case 0x0E:
                            res = packet::Type::DISCONNECT;
                            break;
                        default:
                            res = packet::Type::RESERVED2;
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
                    case packet::Type::RESERVED1:
                        type_string = "Reserved1";
                        break;
                    case packet::Type::CONNECT:
                        type_string = "Connect";
                        break;
                    case packet::Type::CONNACK:
                        type_string = "Connack";
                        break;
                    case packet::Type::PUBLISH:
                        type_string = "Publish";
                        break;
                    case packet::Type::PUBACK:
                        type_string = "Puback";
                        break;
                    case packet::Type::PUBREC:
                        type_string = "Pubrec";
                        break;
                    case packet::Type::PUBCOMP:
                        type_string = "Pubcomp";
                        break;
                    case packet::Type::SUBSCRIBE:
                        type_string = "Subscribe";
                        break;
                    case packet::Type::SUBACK:
                        type_string = "Suback";
                        break;
                    case packet::Type::UNSUBSCRIBE:
                        type_string = "Unsubscribe";
                        break;
                    case packet::Type::UNSUBACK:
                        type_string = "Unsuback";
                        break;
                    case packet::Type::PINGREQ:
                        type_string = "Pingreq";
                        break;
                    case packet::Type::PINGRESP:
                        type_string = "Pingresp";
                        break;
                    case packet::Type::DISCONNECT:
                        type_string = "Disconnect";
                        break;
                    default:
                        type_string = "Reserved2";
                        break;
                }

                output << "header[type:" << type_string << "|" << header.flags( ) << "]";

                return output;
            }
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
