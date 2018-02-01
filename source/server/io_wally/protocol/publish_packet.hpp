#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>
#include <memory>

#include <optional>

#include "io_wally/protocol/common.hpp"

namespace io_wally
{
    namespace protocol
    {
        /// \brief PUBLISH control packet, sent by a client to publish an application message to a \c topic.
        ///
        /// In a PUBLISH control packet the \c fixed \c packet::header shared by all MQTT control packet's is
        /// immediately followed by a \c variable header. This encodes
        ///
        ///  - topic:             a UTF-8 string denoting the \c topic this packet should be published to
        ///  - packet identifier: a 16 bit unsigned integer only present in QoS 1 or QoS 2 publish packets identifying
        ///                       this PUBLISH packet (within the context of a client connection). It MUST not be reused
        ///                       by a client for a different PUBLISH packet until it (the client) has processed the
        ///                       PUBACK (QoS 1) or PUBCOMP (QoS 2) packet sent by a server in response to this PUBLISH
        ///                       packet.
        ///
        /// The \c variable \c header, in turn, is followed by the packet payload. This contains an
        ///
        ///  - application message: an opaque byte sequence
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718037
        struct publish final : public mqtt_packet
        {
           public:
            /// \brief Create a new \c publish instance.
            ///
            /// \param header           Fixed header, common to all MQTT control packets
            /// \param topic            Topic this message should be published to
            /// \param packet_identifier Unsigned 16 bit int identifying this packet: IGNORED IF QOS = 0
            /// \param application_message The message payload, an opaque byte array
            publish( packet::header header,
                     const std::string& topic,
                     const uint16_t packet_identifier,
                     std::vector<uint8_t> application_message )
                : mqtt_packet{std::move( header )},
                  topic_{topic},
                  packet_identifier_{packet_identifier},
                  application_message_{std::move( application_message )}
            {
                assert( header.type( ) == packet::Type::PUBLISH );
            }

            /// \brief Return whether DUP flag is set, i.e. this PUBLISH packet is a duplicate publication.
            ///
            /// \return \c true if DUP flag is set, \c false otherwise
            bool dup( ) const
            {
                return header( ).flags( ).dup( );
            }

            /// \brief Set/unset DUP flag to mark this packet as a duplicate PUBLISH in a QoS 2 publication.
            ///
            /// \param \c dup Whether to mark this packet as a duplicate PUBLISH in a QoS 2 publication
            void dup( const bool dup )
            {
                if ( dup )
                {
                    type_and_flags_ |= 0x08;
                }
                else
                {
                    type_and_flags_ &= ~0x08;
                }
            }

            /// \brief Return quality of service assigned this PUBLISH packet.
            ///
            /// \return quality of service level (At most once, At least once, Exactly once) assigned this packet
            packet::QoS qos( ) const
            {
                return header( ).flags( ).qos( );
            }

            /// \brief Assign quality of service level assigned this PUBLISH packet.
            ///
            /// \param \c new_qos quality of service level to use for this PUBLISH
            void qos( const packet::QoS new_qos )
            {
                const auto old_qos = qos( );
                type_and_flags_ = ( type_and_flags_ & 0xF9 ) | ( static_cast<uint8_t>( new_qos ) << 1 );
                // Adjust remaining length if we change from a QoS that requires a packet identifier to one without or
                // vice versa.
                switch ( old_qos )
                {
                    case packet::QoS::AT_MOST_ONCE:
                        if ( ( new_qos == packet::QoS::AT_LEAST_ONCE ) || ( new_qos == packet::QoS::EXACTLY_ONCE ) )
                        {
                            remaining_length_ += 2;
                        }
                        break;
                    case packet::QoS::AT_LEAST_ONCE:
                    case packet::QoS::EXACTLY_ONCE:
                        if ( new_qos == packet::QoS::AT_MOST_ONCE )
                        {
                            remaining_length_ -= 2;
                        }
                        break;
                    case packet::QoS::RESERVED:
                    default:
                        assert( false );
                }
            }

            /// \brief Return whether RETAIN flag is set, i.e. message should be retained.
            ///
            /// \return \c true if RETAIN flag is set, \c false otherwise
            bool retain( ) const
            {
                return header( ).flags( ).retain( );
            }

            /// \brief Set/unset RETAIN flag, i.e. whether a message should be retained by the the broker.
            ///
            /// \param \c retain Whether message should be retained
            void retain( const bool retain )
            {
                if ( retain )
                {
                    type_and_flags_ |= 0x01;
                }
                else
                {
                    type_and_flags_ &= ~0x01;
                }
            }

            /// \brief Return \c topic to publish this message to.
            ///
            /// \returm topic to publish this message to
            const std::string& topic( ) const
            {
                return topic_;
            }

            /// \brief Test if this PUBLISH packet contains a \c packet \c identifier, i.e. if QoS is NOT QoS 0 (at most
            /// once).
            ///
            /// \return \c true if this packet contains a \c packet \c identifier (QoS != 0), \c false otherwise (QoS =
            ///         0)
            bool has_packet_identifier( ) const
            {
                return ( header( ).flags( ).qos( ) != packet::QoS::AT_MOST_ONCE );
            }

            /// \brief Return packet's \c packet_identifier.
            ///
            /// \return Packet identifier uniquely identifying this packet.
            uint16_t packet_identifier( ) const
            {
                return packet_identifier_;
            }

            /// \brief Return message payload.
            ///
            /// \return message payload, i.e. \c application \c message
            const std::vector<uint8_t>& application_message( ) const
            {
                return application_message_;
            }

            /// \brief Return a string representation to be used in log output.
            ///
            /// \return A string representation to be used in log output
            virtual const std::string to_string( ) const override
            {
                std::ostringstream output;
                output << "publish[";
                if ( ( header( ).flags( ).qos( ) == packet::QoS::AT_LEAST_ONCE ) ||
                     ( header( ).flags( ).qos( ) == packet::QoS::EXACTLY_ONCE ) )
                    output << "pktid:" << packet_identifier_ << "|";
                output << "dup:" << header( ).flags( ).dup( ) << "|qos:" << header( ).flags( ).qos( )
                       << "|ret:" << header( ).flags( ).retain( ) << "|topic:" << topic_
                       << "|msg-size:" << application_message_.size( ) << "]";

                return output.str( );
            }

            std::shared_ptr<publish> with_new_packet_identifier( const std::uint16_t new_packet_identifier ) const
            {
                const packet::header& header_copy = header( );

                return std::make_shared<publish>( header_copy, topic_, new_packet_identifier, application_message_ );
            }

           private:
            const std::string topic_;
            const uint16_t packet_identifier_;
            const std::vector<uint8_t> application_message_;
        };  // struct publish

    }  // namespace protocol
}  // namespace io_wally
