#pragma once

#include <cassert>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include <optional>

#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/unsuback_packet.hpp"

namespace io_wally
{
    namespace protocol
    {
        /// \brief UNSUBSCRIBE control packet, sent by a client to unsubscribe from a list of \c topics.
        ///
        /// In a UNSUBSCRIBE control packet the \c fixed \c packet::header shared by all MQTT control packet's is
        /// immediately followed by a \c variable header. This encodes
        ///
        ///  - packet identifier: a 16 bit unsigned integer identifying this SUBSCRIBE packet (within the context of a
        ///                       client connection). It MUST not be reused by a client for a different SUBSCRIBE packet
        ///                       until it (the client) has processed the SUBACK packet sent by a server in response to
        ///                       this SUBSCRIBE packet.
        ///
        /// The \c variable \c header, in turn, is followed by the packet payload. This contains a
        ///
        ///  - list of topic filters to unsubscribe from
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718063
        struct unsubscribe final : public mqtt_packet
        {
           public:
            /// \brief Create a new \c unsubscribe instance.
            ///
            /// \param header           Fixed header, common to all MQTT control packets
            unsubscribe( const uint32_t remaining_length,
                         const uint16_t packet_identifier,
                         std::vector<std::string> topic_filters )
                : mqtt_packet{( 0x0A << 4 ) | 0x02, remaining_length},
                  packet_identifier_{packet_identifier},
                  topic_filters_{std::move( topic_filters )}
            {
                assert( packet::type_of( type_and_flags_ ) == packet::Type::UNSUBSCRIBE );
            }

            /// \brief Create a new \c unsubscribe instance.
            ///
            /// \param header           Fixed header, common to all MQTT control packets
            unsubscribe( packet::header header,
                         const uint16_t packet_identifier,
                         std::vector<std::string> topic_filters )
                : unsubscribe{header.remaining_length( ), packet_identifier, topic_filters}
            {
                assert( header.type( ) == packet::Type::UNSUBSCRIBE );
            }

            /// \brief Return packet's \c packet_identifier.
            ///
            /// \return Packet identifier uniquely identifying this packet.
            uint16_t packet_identifier( ) const
            {
                return packet_identifier_;
            }

            /// \brief Return UNSUBSCRIBE packet's vector of \c subscriptions.
            ///
            /// \return Vector of \c subscriptions
            const std::vector<std::string>& topic_filters( ) const
            {
                return topic_filters_;
            }

            /// \brief Return a \c unsuback packet for this \c unsubscribe packet.
            ///
            /// \return An \c unsuback packet with packet identifier taken from this \c unsubscribe packet
            std::shared_ptr<const unsuback> ack( ) const
            {
                return std::make_shared<const unsuback>( packet_identifier_ );
            }

            /// \brief Return a string representation to be used in log output.
            ///
            /// \return A string representation to be used in log output
            virtual const std::string to_string( ) const override
            {
                std::ostringstream output;
                output << "unsubscribe[pktid:" << packet_identifier_ << "|";
                for ( auto& subscr : topic_filters_ )
                    output << subscr << " ";
                const long pos = output.tellp( );
                output.seekp( pos - 1 );  // Delete last " "
                output << "]";

                return output.str( );
            }

           private:
            const uint16_t packet_identifier_;
            const std::vector<std::string> topic_filters_;
        };  // struct subscribe

    }  // namespace protocol
}  // namespace io_wally
