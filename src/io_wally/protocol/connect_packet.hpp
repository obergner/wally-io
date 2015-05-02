#pragma once

#include <string>

#include <boost/cstdint.hpp>
#include <boost/optional.hpp>

#include "io_wally/protocol/common.hpp"

using boost::uint8_t;
using boost::optional;

namespace io_wally
{
    namespace protocol
    {

        /// \brief Represents a CONNECT control packet's \c variable header.
        ///
        /// In a CONNECT control packet the \c fixed \c packet::header shared by all MQTT control packet's is
        /// immediately followed by a \c variable header. This encodes
        ///
        ///  - protocol name:   name of protocol used, almost always "MQTT"
        ///  - protocol level:  the \c packet::ProtocolLevel (protocol version), only \c packet::ProtocolLevel::LEVEL4
        ///                     is supported by this implementation
        ///  - username (0/1):  whether the \c connect_payload contains a \c username
        ///  - password (0/1):  whether the \c connect_payload contains a \c password
        ///  - contains_last_will (0/1):        whether \c connect_payload contains a last will message
        ///  - retain_last_will (0/1):  whether a last will message contained in \c connect_payload should be retained
        ///                             by the broker
        ///  - last_will_qos:   delivery \c packet::QoS for last will message contained in \c connect_payload
        ///  - clean_session (0/1):     whether to establish a persistent session (0) or not (1)
        ///  - keep_alive:      For how long in seconds the broker should keep alive the newly established session
        ///                     without the client sending any message
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718030
        struct connect_header
        {
           public:
            /// \brief Create a new \c connect_header instance.
            ///
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

            /// \brief Return \c protocol name, almost always "MQTT"
            ///
            const std::string protocol_name( ) const
            {
                return prot_name_;
            }

            /// \brief Return \c protocol level (version). Only supported version is \c packet::ProtocolLevel::LEVEL4.
            ///
            const packet::ProtocolLevel protocol_level( ) const
            {
                switch ( prot_level_ )
                {
                    case 0x04:
                        return packet::ProtocolLevel::LEVEL4;
                    default:
                        return packet::ProtocolLevel::UNSUPPORTED;
                }
            }

            /// \brief Whether the \c connect_payload contains a username.
            ///
            const bool has_username( ) const
            {
                return ( con_flags_ & 0x80 ) == 0x80;
            }

            /// \brief Whether the \c connect_payload contains a password.
            ///
            const bool has_password( ) const
            {
                return ( con_flags_ & 0x40 ) == 0x40;
            }

            /// \brief Whether the last will message contained in \c connect_payload should be retained.
            ///
            const bool retain_last_will( ) const
            {
                return ( con_flags_ & 0x20 ) == 0x20;
            }

            /// \brief Quality of service for last will message contained in \c connect_payload.
            ///
            /// \see packet::QoS
            const packet::QoS last_will_qos( ) const
            {
                const uint8_t qos_bits = ( con_flags_ & 0x18 ) >> 3;
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

            /// \brief Whether the \c connect_payload contains a last will message.
            ///
            const bool contains_last_will( ) const
            {
                return ( con_flags_ & 0x04 ) == 0x04;
            }

            /// \brief Whether to establish a persistent session (\c false) or not (\c true).
            ///
            const bool clean_session( ) const
            {
                return ( con_flags_ & 0x02 ) == 0x02;
            }

            /// \brief Keep alive timeout in seconds.
            ///
            /// When not receiving a message from a client for at least 1.5 * \c keep_alive_secs() the broker MUST
            /// close the session (provided the keep alive timeout is non-zero).
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
                case packet::QoS::AT_MOST_ONCE:
                    will_qos_string = "At most once";
                    break;
                case packet::QoS::AT_LEAST_ONCE:
                    will_qos_string = "At least once";
                    break;
                case packet::QoS::EXACTLY_ONCE:
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

        /// \brief A CONNECT control packet's payload.
        ///
        ///
        struct connect_payload
        {
           public:
            connect_payload( const char* const client_id,
                             const char* const will_topic,
                             const char* const will_message,
                             const char* const username,
                             const char* const password )
                : client_id_( client_id ),
                  will_topic_( will_topic ? optional<const std::string>( std::string( will_topic ) )
                                          : optional<const std::string>( ) ),
                  will_message_( will_message ? optional<const std::string>( std::string( will_message ) )
                                              : optional<const std::string>( ) ),
                  username_( username ? optional<const std::string>( std::string( username ) )
                                      : optional<const std::string>( ) ),
                  password_( password ? optional<const std::string>( std::string( password ) )
                                      : optional<const std::string>( ) )
            {
                return;
            }

            const std::string& client_id( ) const
            {
                return client_id_;
            }

            const optional<const std::string>& will_topic( ) const
            {
                return will_topic_;
            }

            const optional<const std::string>& will_message( ) const
            {
                return will_message_;
            }

            const optional<const std::string>& username( ) const
            {
                return username_;
            }

            const optional<const std::string>& password( ) const
            {
                return password_;
            }

           private:
            const std::string client_id_;                     // mandatory
            const optional<const std::string> will_topic_;    // MUST be present iff will flag is set
            const optional<const std::string> will_message_;  // MUST be present iff will flag is set
            const optional<const std::string> username_;      // MUST be present iff username flag is set
            const optional<const std::string> password_;      // MUST be present iff password flag is set
        };

        inline std::ostream& operator<<( std::ostream& output, connect_payload const& connect_payload )
        {
            output << "connect_payload[client_id:" << connect_payload.client_id( )
                   << "|will_topic:" << ( connect_payload.will_topic( ) ? *connect_payload.will_topic( ) : "[NULL]" )
                   << "|will_message:" << ( connect_payload.will_message( ) ? "[MESSAGE]" : "[NULL]" )
                   << "|username:" << ( connect_payload.username( ) ? *connect_payload.username( ) : "[NULL]" )
                   << "|password:" << ( connect_payload.password( ) ? "[PROTECTED]" : "[NULL]" ) << "]";

            return output;
        }

        /// \brief CONNECT control packet, sent by a client to establish a new session.
        ///
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
