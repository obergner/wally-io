#pragma once

#include <boost/cstdint.hpp>

#include "io_wally/protocol/common.hpp"

using boost::uint8_t;

namespace io_wally
{
    namespace protocol
    {
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

        struct connect : public mqtt_packet
        {
           public:
            connect( const packet::header& header,
                     const connect_header& connect_header,
                     const connect_payload& payload )
                : mqtt_packet( header ), connect_header_( connect_header ), payload_( payload )
            {
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
            const struct connect_header& connect_header_;
            const struct connect_payload& payload_;
        };
    }  // namespace protocol
}  // namespace io_wally
