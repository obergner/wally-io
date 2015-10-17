#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>

#include <boost/optional.hpp>

#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/subscription.hpp"

namespace io_wally
{
    namespace protocol
    {
        /// \brief SUBSCRIBE control packet, sent by a client to subscribe to a list of \c topics.
        ///
        /// In a SUBSCRIBE control packet the \c fixed \c packet::header shared by all MQTT control packet's is
        /// immediately followed by a \c variable header. This encodes
        ///
        ///  - packet identifier: a 16 bit unsigned integer identifying this SUBSCRIBE packet (within the context of a
        ///                       client connection). It MUST not be reused by a client for a different SUBSCRIBE packet
        ///                       until it (the client) has processed the SUBACK packet sent by a server in response to
        ///                       this SUBSCRIBE packet.
        ///
        /// The \c variable \c header, in turn, is followed by the packet payload. This contains a
        ///
        ///  - list of \c subscriptions: a list of \c topic_filter + \c QoS pairs.
        ///
        /// \see \c subscription
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718063
        struct subscribe : public mqtt_packet
        {
           public:
            /// \brief Create a new \c subscribe instance.
            ///
            /// \param header           Fixed header, common to all MQTT control packets
            subscribe( packet::header header,
                       const uint16_t packet_identifier,
                       std::vector<subscription> subscriptions )
                : mqtt_packet{std::move( header )},
                  packet_identifier_{packet_identifier},
                  subscriptions_{std::move( subscriptions )}
            {
                return;
            }

            /// \brief Return packet's \c packet_identifier.
            ///
            /// \return Packet identifier uniquely identifying this packet.
            uint16_t packet_identifier( ) const
            {
                return packet_identifier_;
            }

            /// \brief Return SUBSCRIBE packet's vector of \c subscriptions.
            ///
            /// \return Vector of \c subscriptions
            const std::vector<subscription>& subscriptions( ) const
            {
                return subscriptions_;
            }

            /// \brief Return a string representation to be used in log output.
            ///
            /// \return A string representation to be used in log output
            virtual const std::string to_string( ) const override
            {
                std::ostringstream output;
                output << "subscribe[" << header( ) << "|pktid:" << packet_identifier_ << "|";
                for ( auto& subscr : subscriptions_ )
                    output << subscr << " ";
                const long pos = output.tellp( );
                output.seekp( pos - 1 );  // Delete last " "
                output << "]";

                return output.str( );
            }

           private:
            const uint16_t packet_identifier_;
            const std::vector<subscription> subscriptions_;
        };  // struct subscribe

    }  // namespace protocol
}  // namespace io_wally
