#pragma once

#include <string>

#include <boost/cstdint.hpp>

using boost::uint8_t;

namespace mqtt
{
    namespace protocol
    {

        const uint8_t CONTROL_PACKET_TYPE_MASK = 0xF0;
        const uint8_t FLAGS_MASK = 0x0F;

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
        }  // namespace packet

        /**
         * CONNECT control packet
         */

        struct connect_header
        {
           public:
            connect_header( const char* const prot_name,
                            const uint8_t prot_level,
                            const uint8_t con_flags,
                            const uint16_t keep_alive_secs )
                : prot_name_( prot_name ),
                  prot_level_( prot_level ),
                  con_flags_( con_flags ),
                  keep_alive_secs_( keep_alive_secs )
            {
            }

            const char* const protocol_name( ) const
            {
                return prot_name_;
            }

            const packet::protocol_level protocol_level( ) const
            {
                switch ( prot_level_ )
                {
                    case 0x04:
                        return packet::LEVEL4;
                    default:
                        return packet::UNSUPPORTED;
                }
            }

            const bool has_username( ) const
            {
                return ( con_flags_ & 0x80 ) == 0x80;
            }

            const bool has_password( ) const
            {
                return ( con_flags_ & 0x40 ) == 0x40;
            }

            const bool retain_last_will( ) const
            {
                return ( con_flags_ & 0x20 ) == 0x20;
            }

            const packet::qos last_will_qos( ) const
            {
                const uint8_t qos_bits = ( con_flags_ & 0x18 ) >> 3;
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

            const bool contains_last_will( ) const
            {
                return ( con_flags_ & 0x04 ) == 0x04;
            }

            const bool clean_session( ) const
            {
                return ( con_flags_ & 0x02 ) == 0x02;
            }

            const uint16_t keep_alive_secs( ) const
            {
                return keep_alive_secs_;
            }

           private:
            const char* const prot_name_;
            const uint8_t prot_level_;
            const uint8_t con_flags_;
            const uint16_t keep_alive_secs_;
        };

        struct connect_payload
        {
           public:
            connect_payload( const char* const client_id,
                             const char* const will_topic,
                             const char* const will_message,
                             const char* const username,
                             const char* const password )
                : client_id_( client_id ),
                  will_topic_( will_topic ),
                  will_message_( will_message ),
                  username_( username ),
                  password_( password )
            {
            }

            const char* const client_id( ) const
            {
                return client_id_;
            }

            const char* const will_topic( ) const
            {
                return will_topic_;
            }

            const char* const will_message( ) const
            {
                return will_message_;
            }

            const char* const username( ) const
            {
                return username_;
            }

            const char* const password( ) const
            {
                return password_;
            }

           private:
            const char* const client_id_;     // mandatory
            const char* const will_topic_;    // MUST be present iff will flag is set
            const char* const will_message_;  // MUST be present iff will flag is set
            const char* const username_;      // MUST be present iff username flag is set
            const char* const password_;      // MUST be present iff password flag is set
        };

        struct connect
        {
           public:
            connect( const packet::header& header,
                     const connect_header& connect_header,
                     const connect_payload& payload )
                : header_( header ), connect_header_( connect_header ), payload_( payload )
            {
            }

            const struct packet::header& header( ) const
            {
                return header_;
            }

            const struct connect_header& connect_header( ) const
            {
                return connect_header_;
            }

            const struct connect_payload& payload( ) const
            {
                return payload_;
            }

           private:
            const struct packet::header& header_;
            const struct connect_header& connect_header_;
            const struct connect_payload& payload_;
        };
    }  // namespace protocol
}  // namespace mqtt
