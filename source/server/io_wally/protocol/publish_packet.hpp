#pragma once

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

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
           public:  // static
            static std::unique_ptr<publish> create( const bool dup,
                                                    const packet::QoS qos,
                                                    const bool retain,
                                                    const std::string& topic,
                                                    const uint16_t packet_identifier,
                                                    std::vector<uint8_t> application_message )
            {
                auto type_and_flags = uint8_t{0x00};
                packet::type_into( packet::Type::PUBLISH, type_and_flags );
                packet::dup_into( dup, type_and_flags );
                packet::qos_into( qos, type_and_flags, 1 );
                packet::retain_into( retain, type_and_flags );

                auto remaining_length =
                    qos == packet::QoS::AT_MOST_ONCE ? uint16_t{0} : uint32_t{2};  // packet_identifier
                remaining_length += ( 2 + topic.length( ) );
                remaining_length += application_message.size( );

                return std::make_unique<publish>( type_and_flags, remaining_length, topic, packet_identifier,
                                                  std::move( application_message ) );
            }

            /**
             * @brief Create a new @c publish instance
             *
             * @param type_and_flags       Fixed header type and flags
             * @param remaining_length     Remaining length of packet
             * @param packet_identifier    Unsigned 16 bit integer identifying this packet (IGNORED IF QoS = 0)
             * @param application_message  The message payload, an opaque byte array
             */
            publish( uint8_t type_and_flags,
                     uint32_t remaining_length,
                     const std::string& topic,
                     const uint16_t packet_identifier,
                     std::vector<uint8_t> application_message )
                : mqtt_packet{type_and_flags, remaining_length},
                  topic_{topic},
                  packet_identifier_{packet_identifier},
                  application_message_{std::move( application_message )}
            {
                assert( packet::type_of( type_and_flags ) == packet::Type::PUBLISH );
            }

           public:
            /// \brief Return whether DUP flag is set, i.e. this PUBLISH packet is a duplicate publication.
            ///
            /// \return \c true if DUP flag is set, \c false otherwise
            bool dup( ) const
            {
                return packet::dup_of( type_and_flags_ );
            }

            /// \brief Set/unset DUP flag to mark this packet as a duplicate PUBLISH in a QoS 2 publication.
            ///
            /// \param \c dup Whether to mark this packet as a duplicate PUBLISH in a QoS 2 publication
            void dup( const bool dup )
            {
                packet::dup_into( dup, type_and_flags_ );
            }

            /// \brief Return quality of service assigned this PUBLISH packet.
            ///
            /// \return quality of service level (At most once, At least once, Exactly once) assigned this packet
            packet::QoS qos( ) const
            {
                return packet::qos_of( type_and_flags_, 1 );
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
                return packet::retain_of( type_and_flags_ );
            }

            /// \brief Set/unset RETAIN flag, i.e. whether a message should be retained by the the broker.
            ///
            /// \param \c retain Whether message should be retained
            void retain( const bool retain )
            {
                packet::retain_into( retain, type_and_flags_ );
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
                return ( qos( ) != packet::QoS::AT_MOST_ONCE );
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
                if ( ( qos( ) == packet::QoS::AT_LEAST_ONCE ) || ( qos( ) == packet::QoS::EXACTLY_ONCE ) )
                    output << "pktid:" << packet_identifier_ << "|";
                output << "dup:" << dup( ) << "|qos:" << qos( ) << "|ret:" << retain( ) << "|topic:" << topic_
                       << "|msg-size:" << application_message_.size( ) << "]";

                return output.str( );
            }

            std::shared_ptr<publish> with_new_packet_identifier( const std::uint16_t new_packet_identifier ) const
            {
                return std::make_shared<publish>( type_and_flags_, remaining_length_, topic_, new_packet_identifier,
                                                  application_message_ );
            }

           private:
            const std::string topic_;
            const uint16_t packet_identifier_;
            const std::vector<uint8_t> application_message_;
        };  // struct publish

    }  // namespace protocol
}  // namespace io_wally
