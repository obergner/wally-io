#pragma once

#include <string>
#include <memory>

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
                return;
            }

            const std::string protocol_name( ) const
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
            const std::string prot_name_;
            const uint8_t prot_level_;
            const uint8_t con_flags_;
            const uint16_t keep_alive_secs_;
        };  /// struct connect_header

        inline std::ostream& operator<<( std::ostream& output, connect_header const& connect_header )
        {
            std::string will_qos_string;
            switch ( connect_header.last_will_qos( ) )
            {
                case packet::AT_MOST_ONCE:
                    will_qos_string = "At most once";
                    break;
                case packet::AT_LEAST_ONCE:
                    will_qos_string = "At least once";
                    break;
                case packet::EXACTLY_ONCE:
                    will_qos_string = "Exactly once";
                    break;
                default:
                    will_qos_string = "Reserved";
                    break;
            }

            output << "connect_header[has_username:" << connect_header.has_username( )
                   << "|has_password:" << connect_header.has_password( )
                   << "|retain_last_will:" << connect_header.retain_last_will( ) << "|last_will_qos:" << will_qos_string
                   << "|contains_last_will:" << connect_header.contains_last_will( )
                   << "|clean_session:" << connect_header.clean_session( )
                   << "|keep_alive_secs:" << connect_header.keep_alive_secs( ) << "]";

            return output;
        }

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
                return client_id_.get( );
            }

            const char* const will_topic( ) const
            {
                return will_topic_.get( );
            }

            const char* const will_message( ) const
            {
                return will_message_.get( );
            }

            const char* const username( ) const
            {
                return username_.get( );
            }

            const char* const password( ) const
            {
                return password_.get( );
            }

           private:
            std::unique_ptr<const char> client_id_;     // mandatory
            std::unique_ptr<const char> will_topic_;    // MUST be present iff will flag is set
            std::unique_ptr<const char> will_message_;  // MUST be present iff will flag is set
            std::unique_ptr<const char> username_;      // MUST be present iff username flag is set
            std::unique_ptr<const char> password_;      // MUST be present iff password flag is set
        };

        inline std::ostream& operator<<( std::ostream& output, connect_payload const& connect_payload )
        {
            output << "connect_payload[client_id:" << connect_payload.client_id( )
                   << "|will_topic:" << ( connect_payload.will_topic( ) ? connect_payload.will_topic( ) : "[NULL]" )
                   << "|will_message:" << ( connect_payload.will_message( ) ? "[MESSAGE]" : "[NULL]" )
                   << "|username:" << ( connect_payload.username( ) ? connect_payload.username( ) : "[NULL]" )
                   << "|password:" << ( connect_payload.password( ) ? "[PROTECTED]" : "[NULL]" ) << "]";

            return output;
        }

        struct connect : public mqtt_packet
        {
           public:
            connect( const packet::header header, const connect_header connect_header, connect_payload payload )
                : mqtt_packet( std::move( header ) ),
                  connect_header_( std::move( connect_header ) ),
                  payload_( std::move( payload ) )
            {
                return;
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
            const struct connect_header connect_header_;
            const struct connect_payload payload_;
        };

        inline std::ostream& operator<<( std::ostream& output, connect const& connect )
        {
            output << "connect[" << connect.header( ) << "|" << connect.connect_header( ) << "|" << connect.payload( )
                   << "]";

            return output;
        }
    }  // namespace protocol
}  // namespace io_wally
