#pragma once

#include <boost/cstdint.hpp>

using boost::uint8_t;

namespace mqtt {
    namespace protocol {

        const uint8_t CONTROL_PACKET_TYPE_MASK = 0xF0;
        const uint8_t FLAGS_MASK = 0x0F;

        typedef enum {
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
        } control_packet_type;

        typedef enum { AT_MOST_ONCE, AT_LEAST_ONCE, EXACTLY_ONCE, RESERVED } quality_of_service;

        struct header_flags {
           public:
            header_flags(const uint8_t& flags) : flags_(flags) {}

            bool dup() const { return (flags_ & 0x08) == 0x08; }

            bool retain() const { return (flags_ & 0x01) == 0x01; }

            const quality_of_service qos() const {
                const uint8_t qos_bits = (flags_ & 0x06) >> 1;
                quality_of_service res;
                switch (qos_bits) {
                    case 0x00:
                        res = AT_MOST_ONCE;
                        break;
                    case 0x01:
                        res = AT_LEAST_ONCE;
                        break;
                    case 0x02:
                        res = EXACTLY_ONCE;
                        break;
                    default:
                        res = RESERVED;
                        break;
                }
                return res;
            }

           private:
            const uint8_t& flags_;
        };

        struct header {
           public:
            header(const uint8_t& type_and_flags) : control_packet_type_and_flags_(type_and_flags) {}

            const control_packet_type type() const {
                const uint8_t type_bits = (control_packet_type_and_flags_ & CONTROL_PACKET_TYPE_MASK) >> 4;
                control_packet_type res;
                switch (type_bits) {
                    case 0x00:
                        res = RESERVED1;
                        break;
                    case 0x01:
                        res = CONNECT;
                        break;
                    case 0x02:
                        res = CONNACK;
                        break;
                    case 0x03:
                        res = PUBLISH;
                        break;
                    case 0x04:
                        res = PUBACK;
                        break;
                    case 0x05:
                        res = PUBREC;
                        break;
                    case 0x06:
                        res = PUBREL;
                        break;
                    case 0x07:
                        res = PUBCOMP;
                        break;
                    case 0x08:
                        res = SUBSCRIBE;
                        break;
                    case 0x09:
                        res = SUBACK;
                        break;
                    case 0x0A:
                        res = UNSUBSCRIBE;
                        break;
                    case 0x0B:
                        res = UNSUBACK;
                        break;
                    case 0x0C:
                        res = PINGREQ;
                        break;
                    case 0x0D:
                        res = PINGRESP;
                        break;
                    case 0x0E:
                        res = DISCONNECT;
                        break;
                    default:
                        res = RESERVED2;
                        break;
                }
                return res;
            }

            const header_flags flags() const { return header_flags(control_packet_type_and_flags_); }

           private:
            const uint8_t& control_packet_type_and_flags_;
        };
    }  // namespace protocol
}  // namespace mqtt
