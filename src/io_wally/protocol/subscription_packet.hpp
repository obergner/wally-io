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
        /// \brief Represents a SUBSCRIBE control packet's \c variable header.
        ///
        /// In a SUBSCRIBE control packet the \c fixed \c packet::header shared by all MQTT control packet's is
        /// immediately followed by a \c variable header. This encodes
        ///
        ///  - packet identifier: a 16 bit unsigned integer identifying this SUBSCRIBE packet (within the context of a
        ///                       client connection). It MUST not be reused by a client for a different SUBSCRIBE packet
        ///                       until it (the client) has processed the SUBACK packet sent by a server in response to
        ///                       this SUBSCRIBE packet.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718063
        struct subscribe_header
        {
           public:
            /// \brief Create a new \c subscribe_header instance.
            ///
            /// \param packet_identifier 16 bit unsigned integer identifying this packet
            subscribe_header( const uint16_t packet_identifier ) : packet_identifier_{packet_identifier}
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

            /// \brief Overload stream output operator for \c subscribe_header.
            ///
            /// Overload stream output operator for \c subscribe_header, primarily to facilitate logging.
            inline friend std::ostream& operator<<( std::ostream& output, subscribe_header const& subscribe_header )
            {
                output << "subscribe_header[pkt_id:" << subscribe_header.packet_identifier_ << "]";
                return output;
            }

           private:
            const uint16_t packet_identifier_;
        };  /// struct subscribe_header

        /// \brief A SUBSCRIBE control packet's payload.
        ///
        /// Contains a list of \c subscriptions, i.e. a list of \c topic_filter + \c QoS pairs.
        ///
        /// \see \c subscription
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718063
        ///
        /// \todo: Consider rule of five
        struct subscribe_payload
        {
           public:
            /// \brief Create a new \c subscribe_payload instance.
            ///
            /// \param subscriptions Vector of \c subscription instances representing topics a client wants to subscribe
            ///                      to
            subscribe_payload( std::vector<subscription> subscriptions ) : subscriptions_{std::move( subscriptions )}
            {
                return;
            }

            /// \brief Return SUBSCRIBE packet's vector of \c subscriptions.
            ///
            /// \return Vector of \c subscriptions
            const std::vector<subscription>& subscriptions( ) const
            {
                return subscriptions_;
            }

            /// \brief Overload stream output operator for \c subscribe_payload.
            ///
            /// Overload stream output operator for \c subscribe_payload, primarily to facilitate logging.
            inline friend std::ostream& operator<<( std::ostream& output, subscribe_payload const& subscribe_payload )
            {
                output << "subscribe_payload[subscriptions:";
                for ( auto& subscr : subscribe_payload.subscriptions_ )
                    output << subscr << "|";
                const long pos = output.tellp( );
                output.seekp( pos - 1 );  // Delete last "|"
                output << "]";

                return output;
            }

           private:
            const std::vector<subscription> subscriptions_;
        };  // struct subscribe_payload

        /// \brief SUBSCRIBE control packet, sent by a client to subscribe to a list of \c topics.
        ///
        /// Combines \c packet::header (fixed header), \c subscribe_header (variable header) and \c subscribe_payload
        /// (SUBSCRIBE packet body).
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718063
        struct subscribe : public mqtt_packet
        {
           public:
            /// \brief Create a new \c subscribe instance.
            ///
            /// \param header           Fixed header, common to all MQTT control packets
            /// \param subscribe_header   Variable header, specific to SUBSCRIBE packet
            /// \param payload          Packet body/payload
            subscribe( packet::header header, subscribe_header subscribe_header, subscribe_payload payload )
                : mqtt_packet{std::move( header )}, subscribe_header_{subscribe_header}, payload_{std::move( payload )}
            {
                return;
            }

            /// \brief Return variable header.
            ///
            /// \return SUBSCRIBE packet variable header
            const struct subscribe_header& subscribe_header( ) const
            {
                return subscribe_header_;
            }

            /// \brief Return packet body/payload
            ///
            /// \return SUBSCRIBE packet payload/body
            const struct subscribe_payload& payload( ) const
            {
                return payload_;
            }

            /// \brief Return a string representation to be used in log output.
            ///
            /// \return A string representation to be used in log output
            virtual const std::string to_string( ) const override
            {
                std::ostringstream output;
                output << "subscribe[" << header( ) << "|" << subscribe_header( ) << "|" << payload( ) << "]";

                return output.str( );
            }

           private:
            const struct subscribe_header subscribe_header_;
            const struct subscribe_payload payload_;
        };  // struct subscribe

    }  // namespace protocol
}  // namespace io_wally
