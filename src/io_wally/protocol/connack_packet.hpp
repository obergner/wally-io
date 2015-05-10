#pragma once

#include <string>
#include <sstream>

#include <boost/cstdint.hpp>
#include <boost/optional.hpp>

#include "io_wally/protocol/common.hpp"

using boost::uint8_t;
using boost::optional;

namespace io_wally
{
    namespace protocol
    {
        /// \brief Allowed connect return codes as defined by MQTT 3.1.1
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718035
        enum class connect_return_code : uint8_t
        {
            /// Connection accepted (OK).
            CONNECTION_ACCEPTED = 0x00,

            /// The Server does not support the level of the MQTT protocol requested by the Client.
            UNACCEPTABLE_PROTOCOL_VERSION,

            /// The Client identifier is correct UTF-8 but not allowed by the Server
            IDENTIFIER_REJECTED,

            /// The Network Connection has been made but the MQTT service is unavailable
            SERVER_UNAVAILABLE,

            /// The data in the user name or password is malformed
            BAD_USERNAME_OR_PASSWORD,

            /// The Client is not authorized to connect
            NOT_AUTHORIZED,

            /// Reserved for future use
            RESERVED
        };

        /// \brief Overload stream output operator for \c connect_return_code.
        ///
        /// Overload stream output operator for \c connect_return_code, primarily to facilitate logging.
        inline std::ostream& operator<<( std::ostream& output, connect_return_code const& return_code )
        {
            switch ( return_code )
            {
                case connect_return_code::CONNECTION_ACCEPTED:
                    output << "Connection accepted";
                    break;
                case connect_return_code::UNACCEPTABLE_PROTOCOL_VERSION:
                    output << "Unacceptable protocol version";
                    break;
                case connect_return_code::IDENTIFIER_REJECTED:
                    output << "Identifier rejected";
                    break;
                case connect_return_code::SERVER_UNAVAILABLE:
                    output << "Server unavailable";
                    break;
                case connect_return_code::BAD_USERNAME_OR_PASSWORD:
                    output << "Bad username or password";
                    break;
                case connect_return_code::NOT_AUTHORIZED:
                    output << "Not authorized";
                    break;
                default:
                    output << "Reserved for future use";
                    break;
            }

            return output;
        }

        /// \brief A CONNACK control packet's variable header.
        ///
        /// Contains a CONNACK control packet's variable header, i.e.
        ///
        ///  - \c session_present (MANDATORY)
        ///  - \c connect_return_code (MANDATORY)
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718035
        struct connack_header
        {
           public:
            /// \brief Create a new \c connack_header instance.
            ///
            /// \param session_present     Whether there already was a persistent session present for this client
            ///                            when it sent CONNECT
            /// \param return_code         Return code in response to CONNECT
            connack_header( const bool session_present, const connect_return_code return_code )
                : session_present_( session_present ), return_code_( return_code )
            {
                return;
            }

            /// \brief Whether there was already a persistent session present for this client when it sent CONNECT.
            ///
            /// \return \c true if client sent a CONNECT packet with \c clean_session set to 0 (\c false) AND there
            ///         was already a persistent session present for this client, \c false otherwise
            ///
            /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718035
            bool is_session_present( ) const
            {
                return session_present_;
            }

            /// \brief Then CONNECT return code.
            ///
            /// \return The CONNECT return code.
            connect_return_code return_code( ) const
            {
                return return_code_;
            }

           private:
            const bool session_present_;
            const connect_return_code return_code_;
        };

        /// \brief Overload stream output operator for \c connack_header.
        ///
        /// Overload stream output operator for \c connack_header, primarily to facilitate logging.
        inline std::ostream& operator<<( std::ostream& output, connack_header const& connack_header )
        {
            output << "connack_header[session_present:" << connack_header.is_session_present( )
                   << "|return_code:" << connack_header.return_code( ) << "]";

            return output;
        }

        /// \brief CONNACK packet, sent in response to a \c connect packet.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718035
        struct connack : public mqtt_ack
        {
           public:
            connack( const bool session_present, const connect_return_code return_code )
                : mqtt_ack( packet::Type::CONNACK, 2 ), connack_header_( session_present, return_code )
            {
                return;
            }

            const struct connack_header& connack_header( ) const
            {
                return connack_header_;
            }

            /// \return A string representation to be used in log output
            virtual const std::string to_string( ) const override
            {
                std::ostringstream output;
                output << "connack[" << header( ) << "]";

                return output.str( );
            }

           private:
            const struct connack_header connack_header_;
        };

    }  // namespace protocol
}  // namespace io_wally
