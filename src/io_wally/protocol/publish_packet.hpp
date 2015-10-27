#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>

#include <boost/optional.hpp>

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

            /// \brief Return \c topic to publish this message to.
            ///
            /// \returm topic to publish this message to
            const std::string& topic( ) const
            {
                return topic_;
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

           private:
            const std::string topic_;
            const uint16_t packet_identifier_;
            const std::vector<uint8_t> application_message_;
        };  // struct publish

    }  // namespace protocol
}  // namespace io_wally
