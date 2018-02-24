#pragma once

#include <cassert>
#include <cstdint>
#include <sstream>
#include <string>

#include <optional>

#include "io_wally/protocol/common.hpp"

namespace io_wally
{
    namespace protocol
    {
        /// \brief Helper struct that knows how to interpret a CONNECT packet's \c connect_flags bit field.
        struct connect_flags final
        {
           public:
            connect_flags( const uint8_t con_flags ) : con_flags_{con_flags}
            {
            }

            /// \brief Whether the \c connect_payload contains a username.
            ///
            /// \return \c true if CONNECT packet contains a \c username field, \c false otherwise
            bool has_username( ) const
            {
                return ( con_flags_ & 0x80 ) == 0x80;
            }

            /// \brief Whether the \c connect_payload contains a password.
            ///
            /// \return \c true if CONNECT packet contains a \c password field, \c false otherwise
            bool has_password( ) const
            {
                return ( con_flags_ & 0x40 ) == 0x40;
            }

            /// \brief Whether the last will message contained in \c connect_payload should be retained.
            ///
            /// \return \c true if broker should retain \c will message if it is published, \c false otherwise
            bool retain_last_will( ) const
            {
                return ( con_flags_ & 0x20 ) == 0x20;
            }

            /// \brief Quality of service for last will message contained in \c connect_payload.
            ///
            /// \return Quality of service mandated for \c will message
            ///
            /// \see packet::QoS
            packet::QoS last_will_qos( ) const
            {
                return packet::qos_of( con_flags_, 3 );
            }

            /// \brief Whether the \c connect_payload contains a last will message.
            ///
            /// \return \c true if CONNECT packet contains a \c will message, \c false otherwise
            bool contains_last_will( ) const
            {
                return ( con_flags_ & 0x04 ) == 0x04;
            }

            /// \brief Whether to establish a persistent session (\c false) or not (\c true).
            ///
            /// \return \c true if broker should NOT establish a persistent session for this client, \c false if it
            ///         SHOULD
            bool clean_session( ) const
            {
                return ( con_flags_ & 0x02 ) == 0x02;
            }

           private:
            const uint8_t con_flags_;
        };  // struct connect_flags
        /// \brief CONNECT control packet, sent by a client to establish a new session.
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
        /// The variable header is followed by CONNECT packet's payload:
        ///
        ///  - \c client_id (MANDATORY)
        ///  - \c will_topic (OPTIONAL)
        ///  - \c will_message (OPTIONAL)
        ///  - \c username (OPTIONAL)
        ///  - \c password (OPTIONAL)
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718028
        struct connect final : public mqtt_packet
        {
           public:
            /**
             * @brief Create a new @c connect instance.
             *
             * @param remaining_length Connect packet's fixed @c header
             * @param prot_name        Protocol name, almost always "MQTT"
             * @param prot_level       Protocol level (protocol version), only @c packet::ProtocolLevel::LEVEL4 aka
             *                         MQTT 3.1.1 is supported by this implementation
             * @param con_flags        CONNECT packet flags, e.g. @c username, @c will qos etc.
             * @param keep_alive_secs  Keep alive period in seconds
             * @param header           Fixed header, common to all MQTT control packets
             * @param client_id        Remote client's unique ID (mandatory)
             * @param will_topic       Topic to publish the remote client's @c will message on, if present (optional)
             * @param will_message     Remote client's @c will message (optional)
             * @param username         Username for authenticating remote client (optional)
             * @param password         Password for authenticating remote client (optional)
             */
            connect( const uint32_t remaining_length,
                     const char* const prot_name,
                     const uint8_t prot_level,
                     const uint8_t con_flags,
                     const uint16_t keep_alive_secs,
                     const char* const client_id,
                     const char* const will_topic,
                     const char* const will_message,
                     const char* const username,
                     const char* const password )
                : mqtt_packet{0x01 << 4, remaining_length},
                  prot_name_{prot_name},
                  prot_level_{prot_level},
                  con_flags_{con_flags},
                  keep_alive_secs_{keep_alive_secs},
                  client_id_{client_id},
                  will_topic_{will_topic ? std::optional<const std::string>( std::string( will_topic ) )
                                         : std::optional<const std::string>( )},
                  will_message_{will_message ? std::optional<const std::string>( std::string( will_message ) )
                                             : std::optional<const std::string>( )},
                  username_{username ? std::optional<const std::string>( std::string( username ) )
                                     : std::optional<const std::string>( )},
                  password_{password ? std::optional<const std::string>( std::string( password ) )
                                     : std::optional<const std::string>( )}
            {
                assert( packet::type_of( type_and_flags_ ) == packet::Type::CONNECT );
            }

            /// \brief Create a new \c connect instance.
            ///
            /// \param header           Connect packet's fixed \c header
            /// \param prot_name        Protocol name, almost always "MQTT"
            /// \param prot_level       Protocol level (protocol version), only \c packet::ProtocolLevel::LEVEL4 aka
            ///                         MQTT 3.1.1 is supported by this implementation
            /// \param con_flags        CONNECT packet flags, e.g. \c username, \c will qos etc.
            /// \param keep_alive_secs  Keep alive period in seconds
            /// \param header           Fixed header, common to all MQTT control packets
            /// \param client_id        Remote client's unique ID (mandatory)
            /// \param will_topic       Topic to publish the remote client's \c will message on, if present (optional)
            /// \param will_message     Remote client's \c will message (optional)
            /// \param username         Username for authenticating remote client (optional)
            /// \param password         Password for authenticating remote client (optional)
            connect( packet::header header,
                     const char* const prot_name,
                     const uint8_t prot_level,
                     const uint8_t con_flags,
                     const uint16_t keep_alive_secs,
                     const char* const client_id,
                     const char* const will_topic,
                     const char* const will_message,
                     const char* const username,
                     const char* const password )
                : connect{header.remaining_length( ),
                          prot_name,
                          prot_level,
                          con_flags,
                          keep_alive_secs,
                          client_id,
                          will_topic,
                          will_message,
                          username,
                          password}
            {
                assert( header.type( ) == packet::Type::CONNECT );
            }

            /// \brief Return \c protocol name, almost always "MQTT"
            ///
            /// \return Protocol name used by this CONNECT packet, almost always "MQTT"
            const std::string protocol_name( ) const
            {
                return prot_name_;
            }

            /// \brief Return \c protocol level (version). Only supported version is \c packet::ProtocolLevel::LEVEL4.
            ///
            /// \return Protocol level, only supported value is \c Level4, aka MQTT 3.1.1
            packet::ProtocolLevel protocol_level( ) const
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
            /// \return \c true if CONNECT packet contains a \c username field, \c false otherwise
            bool has_username( ) const
            {
                return con_flags_.has_username( );
            }

            /// \brief Whether the \c connect_payload contains a password.
            ///
            /// \return \c true if CONNECT packet contains a \c password field, \c false otherwise
            bool has_password( ) const
            {
                return con_flags_.has_password( );
            }

            /// \brief Whether the last will message contained in \c connect_payload should be retained.
            ///
            /// \return \c true if broker should retain \c will message if it is published, \c false otherwise
            bool retain_last_will( ) const
            {
                return con_flags_.retain_last_will( );
            }

            /// \brief Quality of service for last will message contained in \c connect_payload.
            ///
            /// \return Quality of service mandated for \c will message
            ///
            /// \see packet::QoS
            packet::QoS last_will_qos( ) const
            {
                return con_flags_.last_will_qos( );
            }

            /// \brief Whether the \c connect_payload contains a last will message.
            ///
            /// \return \c true if CONNECT packet contains a \c will message, \c false otherwise
            bool contains_last_will( ) const
            {
                return con_flags_.contains_last_will( );
            }

            /// \brief Whether to establish a persistent session (\c false) or not (\c true).
            ///
            /// \return \c true if broker should NOT establish a persistent session for this client, \c false if it
            ///         SHOULD
            bool clean_session( ) const
            {
                return con_flags_.clean_session( );
            }

            /// \brief Keep alive timeout in seconds.
            ///
            /// When not receiving a message from a client for at least 1.5 * \c keep_alive_secs() the broker MUST
            /// close the session (provided the keep alive timeout is non-zero).
            ///
            /// \return Keep alive timeout in seconds
            uint16_t keep_alive_secs( ) const
            {
                return keep_alive_secs_;
            }

            /// \brief Return remote client's unique ID
            ///
            /// \return Remote client ID
            const std::string& client_id( ) const
            {
                return client_id_;
            }

            /// \brief Return last will topic (if present)
            ///
            /// \return Topic this client's last will message should be published to (if present)
            const std::optional<const std::string>& will_topic( ) const
            {
                return will_topic_;
            }

            /// \brief Return last will message (if present)
            ///
            /// \return Message to publish in case this remote client "dies", i.e. unexpectedly disconnects (if present)
            const std::optional<const std::string>& will_message( ) const
            {
                return will_message_;
            }

            /// \brief Return \c username (if present)
            ///
            /// \return Username authenticating remote client (if present)
            const std::optional<const std::string>& username( ) const
            {
                return username_;
            }

            /// \brief Return \c password (if present)
            ///
            /// \return Password authenticating remote client (if present)
            const std::optional<const std::string>& password( ) const
            {
                return password_;
            }

            /// \brief Return a string representation to be used in log output.
            ///
            /// \return A string representation to be used in log output
            virtual const std::string to_string( ) const override
            {
                std::ostringstream output;
                output << "connect[cltid:" << client_id_ << "|lwt_ret:" << retain_last_will( )
                       << "|lwt_qos:" << last_will_qos( ) << "|clean_sess:" << clean_session( )
                       << "|keep_alive:" << keep_alive_secs( )
                       << "|lwt_top:" << ( will_topic_ ? *will_topic_ : "[NULL]" )
                       << "|lwt_msg:" << ( will_message_ ? "[MESSAGE]" : "[NULL]" )
                       << "|usr:" << ( username_ ? *username_ : "[NULL]" )
                       << "|pwd:" << ( password_ ? "[PROTECTED]" : "[NULL]" ) << "]";

                return output.str( );
            }

           private:
            const std::string prot_name_;
            const uint8_t prot_level_;
            const struct connect_flags con_flags_;
            const uint16_t keep_alive_secs_;
            const std::string client_id_;                          // mandatory
            const std::optional<const std::string> will_topic_;    // MUST be present iff will flag is set
            const std::optional<const std::string> will_message_;  // MUST be present iff will flag is set
            const std::optional<const std::string> username_;      // MUST be present iff username flag is set
            const std::optional<const std::string> password_;      // MUST be present iff password flag is set
        };                                                         // struct connect

    }  // namespace protocol
}  // namespace io_wally
