#pragma once

#include <cstdint>
#include <string>
#include <sstream>

#include <boost/optional.hpp>

#include "io_wally/protocol/common.hpp"

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
            /// \param prot_name        Protocol name, almost always "MQTT"
            /// \param prot_level       Protocol level (protocol version), only \c packet::ProtocolLevel::LEVEL4 aka
            ///                         MQTT 3.1.1 is supported by this implementation
            /// \param con_flags        CONNECT packet flags, e.g. \c username, \c will qos etc.
            /// \param keep_alive_secs  Keep alive period in seconds
            connect_header( const char* const prot_name,
                            const uint8_t prot_level,
                            const uint8_t con_flags,
                            const uint16_t keep_alive_secs )
                : prot_name_{prot_name},
                  prot_level_{prot_level},
                  con_flags_{con_flags},
                  keep_alive_secs_{keep_alive_secs}
            {
                return;
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

            /// \brief Overload stream output operator for \c connect_header.
            ///
            /// Overload stream output operator for \c connect_header, primarily to facilitate logging.
            inline friend std::ostream& operator<<( std::ostream& output, connect_header const& connect_header )
            {
                output << "connect_header[has_username:" << connect_header.has_username( )
                       << "|has_password:" << connect_header.has_password( )
                       << "|retain_last_will:" << connect_header.retain_last_will( )
                       << "|last_will_qos:" << connect_header.last_will_qos( )
                       << "|contains_last_will:" << connect_header.contains_last_will( )
                       << "|clean_session:" << connect_header.clean_session( )
                       << "|keep_alive_secs:" << connect_header.keep_alive_secs( ) << "]";

                return output;
            }

           private:
            const std::string prot_name_;
            const uint8_t prot_level_;
            const uint8_t con_flags_;
            const uint16_t keep_alive_secs_;
        };  /// struct connect_header

        /// \brief A CONNECT control packet's payload.
        ///
        /// Contains a CONNECT control packet's payload, i.e.
        ///
        ///  - \c client_id (MANDATORY)
        ///  - \c will_topic (OPTIONAL)
        ///  - \c will_message (OPTIONAL)
        ///  - \c username (OPTIONAL)
        ///  - \c password (OPTIONAL)
        ///
        struct connect_payload
        {
           public:
            /// \brief Create a new \c connect_payload instance.
            ///
            /// \param client_id     Remote client's unique ID (mandatory)
            /// \param will_topic    Topic to publish the remote client's \c will message on, if present (optional)
            /// \param will_message  Remote client's \c will message (optional)
            /// \param username      Username for authenticating remote client (optional)
            /// \param password      Password for authenticating remote client (optional)
            connect_payload( const char* const client_id,
                             const char* const will_topic,
                             const char* const will_message,
                             const char* const username,
                             const char* const password )
                : client_id_{client_id},
                  will_topic_{will_topic ? boost::optional<const std::string>( std::string( will_topic ) )
                                         : boost::optional<const std::string>( )},
                  will_message_{will_message ? boost::optional<const std::string>( std::string( will_message ) )
                                             : boost::optional<const std::string>( )},
                  username_{username ? boost::optional<const std::string>( std::string( username ) )
                                     : boost::optional<const std::string>( )},
                  password_{password ? boost::optional<const std::string>( std::string( password ) )
                                     : boost::optional<const std::string>( )}
            {
                return;
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
            const boost::optional<const std::string>& will_topic( ) const
            {
                return will_topic_;
            }

            /// \brief Return last will message (if present)
            ///
            /// \return Message to publish in case this remote client "dies", i.e. unexpectedly disconnects (if present)
            const boost::optional<const std::string>& will_message( ) const
            {
                return will_message_;
            }

            /// \brief Return \c username (if present)
            ///
            /// \return Username authenticating remote client (if present)
            const boost::optional<const std::string>& username( ) const
            {
                return username_;
            }

            /// \brief Return \c password (if present)
            ///
            /// \return Password authenticating remote client (if present)
            const boost::optional<const std::string>& password( ) const
            {
                return password_;
            }

            /// \brief Overload stream output operator for \c connect_payload.
            ///
            /// Overload stream output operator for \c connect_payload, primarily to facilitate logging.
            inline friend std::ostream& operator<<( std::ostream& output, connect_payload const& connect_payload )
            {
                output << "connect_payload[client_id:" << connect_payload.client_id( ) << "|will_topic:"
                       << ( connect_payload.will_topic( ) ? *connect_payload.will_topic( ) : "[NULL]" )
                       << "|will_message:" << ( connect_payload.will_message( ) ? "[MESSAGE]" : "[NULL]" )
                       << "|username:" << ( connect_payload.username( ) ? *connect_payload.username( ) : "[NULL]" )
                       << "|password:" << ( connect_payload.password( ) ? "[PROTECTED]" : "[NULL]" ) << "]";

                return output;
            }

           private:
            const std::string client_id_;                            // mandatory
            const boost::optional<const std::string> will_topic_;    // MUST be present iff will flag is set
            const boost::optional<const std::string> will_message_;  // MUST be present iff will flag is set
            const boost::optional<const std::string> username_;      // MUST be present iff username flag is set
            const boost::optional<const std::string> password_;      // MUST be present iff password flag is set
        };                                                           // struct connect_payload

        /// \brief CONNECT control packet, sent by a client to establish a new session.
        ///
        /// Combines \c packet::header (fixed header), \c connect_header (variable header) and \c connect_payload
        /// (CONNECT packet body).
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718028
        struct connect : public mqtt_packet
        {
           public:
            /// \brief Create a new \c connect instance.
            ///
            /// \param header           Fixed header, common to all MQTT control packets
            /// \param connect_header   Variable header, specific to CONNECT packet
            /// \param payload          Packet body/payload
            connect( packet::header header, connect_header connect_header, connect_payload payload )
                : mqtt_packet{std::move( header )},
                  connect_header_{std::move( connect_header )},
                  payload_{std::move( payload )}
            {
                return;
            }

            /// \brief Return variable header.
            ///
            /// \return CONNECT packet variable header
            const struct connect_header& connect_header( ) const
            {
                return connect_header_;
            }

            /// \brief Return packet body/payload
            ///
            /// \return CONNECT packet payload/body
            const struct connect_payload& payload( ) const
            {
                return payload_;
            }

            /// \brief Return a string representation to be used in log output.
            ///
            /// \return A string representation to be used in log output
            virtual const std::string to_string( ) const override
            {
                std::ostringstream output;
                output << "connect[" << header( ) << "|" << connect_header( ) << "|" << payload( ) << "]";

                return output.str( );
            }

           private:
            const struct connect_header connect_header_;
            const struct connect_payload payload_;
        };  // struct connect

    }  // namespace protocol
}  // namespace io_wally
