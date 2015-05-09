#pragma once

#include <iostream>

#include <boost/cstdint.hpp>

using boost::uint8_t;

namespace io_wally
{
    /// \brief Namespace for MQTT Control Packet implementations.
    ///
    /// This namespace models MQTT "domain" classes, i.e. all MQTT Control Packets:
    ///
    ///  - CONNECT
    ///  - CONNACK
    ///  - PUBLISH
    ///  - PUBACK
    ///  - ...
    ///
    namespace protocol
    {
        ///
        /// Namespace for data types shared across all MQTT packets:
        ///
        ///  - quality of service
        ///  - control packet type
        ///  - protocol level
        ///  - header flags
        ///  - fixed header
        ///
        ///
        namespace packet
        {

            const uint8_t CONTROL_PACKET_TYPE_MASK = 0xF0;

            /// \brief Quality of service
            ///
            /// Contract on delivery guarantuees for published messages:
            ///
            enum class QoS : int
            {
                /// A published message is delivered at most once to each subscriber. A subscriber may not see all
                /// messages published to a topic it is subscribed to.
                AT_MOST_ONCE,

                /// A published message is delivered at least once to each subscriber. A subscriber may see one and the
                /// same message delivered to it more than once.
                AT_LEAST_ONCE,

                /// A published message is delivered exactly once to each subscriber. Often the most desirable but
                /// also the most expensive guarantuee.
                EXACTLY_ONCE,

                /// Reserved for future use. MUST NOT BE USED.
                RESERVED
            };

            /// \brief Type of MQTT control packet.
            ///
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

            /// \brief Protocol level, a.k.a. protocol version.
            ///
            enum class ProtocolLevel : int
            {
                /// Protocol level 4, the protocol level that identifies MQTT v. 3.1.1.
                LEVEL4 = 0x04,

                /// All protocol levels that are not level 4.
                UNSUPPORTED
            };

            /// \brief Flags declared in an MQTT control packet's \c fixed header.
            ///
            /// An MQTT PUBLISH packet contains a set of standard flags in bits 0 - 3 of its first byte:
            ///
            ///  - dup:         whether this is the first (0) delivery attempt or not (1)
            ///  - retain:      whether the broker should retain this message for delivery to future subscribers to
            ///                 its topic
            ///  - qos:         quality of service for this message
            struct header_flags
            {
               public:
                /// \brief Construct a new \c header_flags instance from bits 0 - 3 in \c flags.
                header_flags( const uint8_t flags ) : flags_( flags )
                {
                    return;
                }

                /// Whether this is the first (\c false) delivery attempt or not (\c true)
                bool dup( ) const
                {
                    return ( flags_ & 0x08 ) == 0x08;
                }

                /// Whether the broker should retain this message for delivery to future subscribers to its topic.
                bool retain( ) const
                {
                    return ( flags_ & 0x01 ) == 0x01;
                }

                /// Message delivery quality of service.
                ///
                /// \see QoS
                packet::QoS qos( ) const
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
                /// Fields
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

            /// \brief Represents an MQTT control packet's \c fixed header.
            ///
            /// Each MQTT control packet starts with a variable length \c fixed header encoding
            ///
            ///  - control packet type (CONNECT, CONNACK, ...) in byte 1, bits, 4 - 7
            ///  - flags (only relevant for packet::PacketType::PUBLISH) in byte 1, bits 0 - 3
            ///  - remaining length of control packet, excluding fixed header; takes between 1 and 4 bytes on the wire
            ///
            /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718020
            struct header
            {
               public:
                /// \brief Create a new \c header instance from the supplied \c type_and_flags and \c
                /// remaining_length.
                header( const uint8_t type_and_flags, const uint32_t remaining_length )
                    : control_packet_type_and_flags_( type_and_flags ), remaining_length_( remaining_length )
                {
                    return;
                }

                /// \brief Return this control packet's \c type.
                ///
                /// \see packet::PacketType
                packet::Type type( ) const
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

                /// \brief Return this control packet' header flags (only relevant for PUBLISH control packets).
                ///
                /// \see packet::header_flags
                const packet::header_flags flags( ) const
                {
                    return packet::header_flags( control_packet_type_and_flags_ );
                }

                /// \brief Return this control packet's remaining length in bytes on the wire, excluding fixed header.
                ///
                uint32_t remaining_length( ) const
                {
                    return remaining_length_;
                }

               private:
                /// Fields
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

        /// \brief Base class for all MQTT control packets.
        ///
        struct mqtt_packet
        {
           public:
            /// \brief Create a new \c mqtt_packet instance from the supplied \c packet::header.
            ///
            /// \param header This MQTT packet's packet::header
            mqtt_packet( struct packet::header header ) : header_( std::move( header ) )
            {
                return;
            }

            /// \brief Return this \c mqtt_packet's \c packet::header.
            ///
            /// \return This MQTT packet's packet::header
            const struct packet::header& header( ) const
            {
                return header_;
            }

            /// \brief Return a string representation of this packet to be used e.g. in log output.
            ///
            /// \return A string representation of this MQTT packet suitable for log output.
            virtual const std::string to_string( ) const = 0;

           private:
            /// Fields
            const struct packet::header header_;
        };

        inline std::ostream& operator<<( std::ostream& output, mqtt_packet const& mqtt_packet )
        {
            output << mqtt_packet.to_string( );

            return output;
        }

    }  // namespace protocol
}  // namespace io_wally
