#pragma once

#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

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

            /// \brief Quality of service
            ///
            /// Contract on delivery guarantuees for published messages:
            ///
            enum class QoS : uint8_t
            {
                /// A published message is delivered at most once to each subscriber. A subscriber may not see all
                /// messages published to a topic it is subscribed to.
                AT_MOST_ONCE = 0,

                /// A published message is delivered at least once to each subscriber. A subscriber may see one and the
                /// same message delivered to it more than once.
                AT_LEAST_ONCE = 1,

                /// A published message is delivered exactly once to each subscriber. Often the most desirable but
                /// also the most expensive guarantuee.
                EXACTLY_ONCE = 2,

                /// Reserved for future use. MUST NOT BE USED.
                RESERVED
            };

            /**
             * @brief Determine @c QoS encoded in @c byte and return it
             *
             * @param byte       A byte containing an encoded QoS
             * @param left_shift Optional left shift to apply
             * @return           Decoded @c QoS
             */
            inline QoS qos_of( const uint8_t byte, const uint8_t left_shift = 0 )
            {
                assert( left_shift < 7 );

                packet::QoS res;
                switch ( ( byte >> left_shift ) & 0x03 )
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

            /**
             * @brief Encode @c qos into @c byte, optionally shifting by @c left_shift
             *
             * @param qos         @c QoS to encode
             * @param byte        Byte to encode into
             * @param left_shift  Optional left shift to apply
             */
            inline void qos_into( const QoS qos, uint8_t& byte, const uint8_t left_shift = 0 )
            {
                assert( left_shift < 7 );

                uint8_t qos_mask = 0x00;
                switch ( qos )
                {
                    case packet::QoS::AT_MOST_ONCE:
                        qos_mask = ( 0x00 << left_shift );
                        break;
                    case packet::QoS::AT_LEAST_ONCE:
                        qos_mask = ( 0x01 << left_shift );
                        break;
                    case packet::QoS::EXACTLY_ONCE:
                        qos_mask = ( 0x02 << left_shift );
                        break;
                    case packet::QoS::RESERVED:
                        qos_mask = ( 0x0F << left_shift );
                        break;
                    default:
                        assert( false );
                        break;
                }

                byte &= 0xFF - ( 0x03 << left_shift );
                byte |= qos_mask;
            }

            /// \brief Overload stream output operator for \c packet::QoS.
            ///
            /// Overload stream output operator for \c packet::QoS, primarily meant to facilitate logging.
            inline std::ostream& operator<<( std::ostream& output, packet::QoS const& qos )
            {
                std::string qos_string;
                switch ( qos )
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
                    case packet::QoS::RESERVED:
                        qos_string = "Reserved";
                        break;
                    default:
                        qos_string = "Reserved";
                        break;
                }
                output << qos_string;

                return output;
            }

            /// \brief Type of MQTT control packet.
            ///
            enum class Type : uint8_t
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

            /// \brief Bit mask for extracting bits 4-7, the bits encoding a packet's \c Type, out of a byte.
            constexpr const uint8_t CONTROL_PACKET_TYPE_MASK = 0xF0;

            /// \brief Determine type of control packet encoded in \c control_packet_type_and_flags and return it as
            ///         \c Type enum.
            ///
            /// The first byte in each MQTT control packet encodes in bits 4-7 that packet's \c Type. Extract those
            /// bits and convert them into a \c Type enum instance.
            ///
            /// \param control_packet_type_and_flags    First byte in an encoded MQTT packet
            /// \return         A \c Type instance representing the type of MQTT packet encoded in
            ///                 \c control_packet_type_and_flags
            inline Type type_of( const uint8_t control_packet_type_and_flags )
            {
                const uint8_t type_bits = ( control_packet_type_and_flags & CONTROL_PACKET_TYPE_MASK ) >> 4;
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

            /// \brief Write \c type into \c control_packet_type_and_flags, i.e. replace bits 4-7 with bits
            ///         representing \c type.
            ///
            /// \param type     \c Type to write
            /// \param control_packet_type_and_flags    Byte to write \c type into
            inline void type_into( const Type type, uint8_t& control_packet_type_and_flags )
            {
                uint8_t type_mask = 0x00;
                switch ( type )
                {
                    case packet::Type::RESERVED1:
                        type_mask = ( 0x00 << 4 );
                        break;
                    case packet::Type::CONNECT:
                        type_mask = ( 0x01 << 4 );
                        break;
                    case packet::Type::CONNACK:
                        type_mask = ( 0x02 << 4 );
                        break;
                    case packet::Type::PUBLISH:
                        type_mask = ( 0x03 << 4 );
                        break;
                    case packet::Type::PUBACK:
                        type_mask = ( 0x04 << 4 );
                        break;
                    case packet::Type::PUBREC:
                        type_mask = ( 0x05 << 4 );
                        break;
                    case packet::Type::PUBREL:
                        type_mask = ( 0x06 << 4 );
                        break;
                    case packet::Type::PUBCOMP:
                        type_mask = ( 0x07 << 4 );
                        break;
                    case packet::Type::SUBSCRIBE:
                        type_mask = ( 0x08 << 4 );
                        break;
                    case packet::Type::SUBACK:
                        type_mask = ( 0x09 << 4 );
                        break;
                    case packet::Type::UNSUBSCRIBE:
                        type_mask = ( 0x0A << 4 );
                        break;
                    case packet::Type::UNSUBACK:
                        type_mask = ( 0x0B << 4 );
                        break;
                    case packet::Type::PINGREQ:
                        type_mask = ( 0x0C << 4 );
                        break;
                    case packet::Type::PINGRESP:
                        type_mask = ( 0x0D << 4 );
                        break;
                    case packet::Type::DISCONNECT:
                        type_mask = ( 0x0E << 4 );
                        break;
                    case packet::Type::RESERVED2:
                        type_mask = ( 0x0F << 4 );
                        break;
                    default:
                        assert( false );
                        break;
                }

                control_packet_type_and_flags &= 0x0F;  // Set bits 4-7 to 0.
                control_packet_type_and_flags |= type_mask;
            }

            inline bool dup_of( const uint8_t control_packet_type_and_flags )
            {
                return ( control_packet_type_and_flags & 0x08 ) == 0x08;
            }

            inline void dup_into( const bool dup, uint8_t& control_packet_type_and_flags )
            {
                if ( dup )
                {
                    control_packet_type_and_flags |= 0x08;
                }
                else
                {
                    control_packet_type_and_flags &= ~0x08;
                }
            }

            inline bool retain_of( const uint8_t control_packet_type_and_flags )
            {
                return ( control_packet_type_and_flags & 0x01 ) == 0x01;
            }

            inline void retain_into( const bool retain, uint8_t& control_packet_type_and_flags )
            {
                if ( retain )
                {
                    control_packet_type_and_flags |= 0x01;
                }
                else
                {
                    control_packet_type_and_flags &= ~0x01;
                }
            }

            /// \brief Protocol level, a.k.a. protocol version.
            ///
            enum class ProtocolLevel : int
            {
                /// Protocol level 4, the protocol level that identifies MQTT v. 3.1.1.
                LEVEL4 = 0x04,

                /// All protocol levels that are not level 4.
                UNSUPPORTED
            };

            constexpr const uint32_t MAX_ALLOWED_PACKET_LENGTH = 268435455;

            constexpr const uint32_t MAX_FIXED_HEADER_LENGTH = 5;

            /**
             * @brief Compute an MQTT packet's total lenght in bytes on the wire given its @c remaining_length
             *
             * An MQTT packet's total length in bytes on the wire is simply the sum of its fixed header's lenght
             * plus its @c remaining_length.
             */
            inline uint32_t total_length( const uint32_t remaining_length )
            {
                uint32_t rem_length_bytes;
                if ( remaining_length <= 127 )
                    rem_length_bytes = 1;
                else if ( remaining_length <= 16383 )
                    rem_length_bytes = 2;
                else if ( remaining_length <= 2097151 )
                    rem_length_bytes = 3;
                else
                    rem_length_bytes = 4;

                return 1 + rem_length_bytes + remaining_length;
            }
        }  // namespace packet

        /// \brief Base class for all MQTT control packets.
        ///
        struct mqtt_packet
        {
           public:
            using ptr = std::shared_ptr<const mqtt_packet>;

            /**
             * @brief Return this @c mqtt_packet's @c packet::Type
             *
             * @return This @c mqtt_packet's @c packet::Type
             */
            packet::Type type( ) const
            {
                return packet::type_of( type_and_flags_ );
            }

            /**
             * @brief Return control packet type and flags in "raw" form, i.e. as a byte.
             */
            uint8_t type_and_flags( ) const
            {
                return type_and_flags_;
            }

            /**
             * @brief Return this @c mqtt_packet's remaining length
             *
             * @return This @c mqtt_packet's remaining length
             */
            uint32_t remaining_length( ) const
            {
                return remaining_length_;
            }

            /// \brief Return this MQTT packet's total length in bytes on the wire.
            ///
            /// An MQTT packet's total length in bytes on the wire is simply the sum of its fixed header's lenght
            /// plus its \c remaining_length().
            uint32_t total_length( ) const
            {
                return packet::total_length( remaining_length_ );
            }

            /// \brief Return a string representation of this packet to be used e.g. in log output.
            ///
            /// \return A string representation of this MQTT packet suitable for log output.
            virtual const std::string to_string( ) const = 0;

            /// \brief Overload stream output operator for \c mqtt_packet.
            ///
            /// Overload stream output operator for \c mqtt_packet, primarily meant to facilitate logging.
            inline friend std::ostream& operator<<( std::ostream& output, mqtt_packet const& mqtt_packet )
            {
                output << mqtt_packet.to_string( );

                return output;
            }

           protected:
            /**
             * @brief Create a new @c mqtt_packet instance from @c type_and_flags and @c remaining_length.
             *
             * @param type_and_flags   Byte containing packet type and fixed header flags
             * @param remaining_length Remaining length of packet
             */
            mqtt_packet( const uint8_t type_and_flags, const uint32_t remaining_length )
                : type_and_flags_{type_and_flags}, remaining_length_{remaining_length}
            {
                return;
            }

           protected:
            uint8_t type_and_flags_;
            uint32_t remaining_length_;
        };

        /// \brief Base class for MQTT acks.
        ///
        struct mqtt_ack : public mqtt_packet
        {
           protected:
            mqtt_ack( const packet::Type type, const uint32_t remaining_length, const uint8_t flags = 0x00 )
                : mqtt_packet{flags, remaining_length}
            {
                packet::type_into( type, type_and_flags_ );
            }
        };  // struct mqtt_ack

    }  // namespace protocol
}  // namespace io_wally
