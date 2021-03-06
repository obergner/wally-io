#pragma once

#include <cassert>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include <optional>

#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/suback_packet.hpp"
#include "io_wally/protocol/subscription.hpp"

namespace io_wally::protocol
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
    struct subscribe final : public mqtt_packet
    {
       public:
        /// \brief Create a new \c subscribe instance.
        ///
        /// \param header           Fixed header, common to all MQTT control packets
        subscribe( const uint32_t remaining_length,
                   const uint16_t packet_identifier,
                   std::vector<subscription> subscriptions )
            : mqtt_packet{( 0x08 << 4 ) | 0x02, remaining_length},
              packet_identifier_{packet_identifier},
              subscriptions_{std::move( subscriptions )}
        {
            assert( packet::type_of( type_and_flags_ ) == packet::Type::SUBSCRIBE );
        }

        /// \brief Return packet's \c packet_identifier.
        ///
        /// \return Packet identifier uniquely identifying this packet.
        [[nodiscard]] auto packet_identifier( ) const -> uint16_t
        {
            return packet_identifier_;
        }

        /// \brief Return SUBSCRIBE packet's vector of \c subscriptions.
        ///
        /// \return Vector of \c subscriptions
        [[nodiscard]] auto subscriptions( ) const -> const std::vector<subscription>&
        {
            return subscriptions_;
        }

        /// \brief Return a \c suback packet representing complete failure to process this \c subscribe packet.
        ///
        /// This is a convenience method for creating responses to a \c subscribe packet when processing that \c
        /// subscribe packet failed entirely, i.e. due to network or server failure.
        ///
        /// \return A \c suback packet with all its \c suback_return_codes set to \c suback_return_code::FAILURE
        [[nodiscard]] auto fail( ) const -> std::shared_ptr<const suback>
        {
            std::vector<suback_return_code> rcs{};
            rcs.assign( subscriptions_.size( ), suback_return_code::FAILURE );

            return std::make_shared<const suback>( packet_identifier_, rcs );
        }

        /// \brief Return a \c suback packet representing complete success to process this \c subscribe packet.
        ///
        /// This is a convenience method for creating responses to a \c subscribe packet when we simply want to
        /// confirm all subscription requests including desired maximum QoS to client.
        ///
        /// \return A \c suback packet with all its \c suback_return_codes set to values desired by client
        [[nodiscard]] auto succeed( ) const -> std::shared_ptr<const suback>
        {
            std::vector<suback_return_code> rcs{};
            for ( auto& subscr : subscriptions_ )
            {
                switch ( subscr.maximum_qos( ) )
                {
                    case protocol::packet::QoS::AT_MOST_ONCE:
                        rcs.push_back( protocol::suback_return_code::MAXIMUM_QOS0 );
                        break;
                    case protocol::packet::QoS::AT_LEAST_ONCE:
                        rcs.push_back( protocol::suback_return_code::MAXIMUM_QOS1 );
                        break;
                    case protocol::packet::QoS::EXACTLY_ONCE:
                        rcs.push_back( protocol::suback_return_code::MAXIMUM_QOS2 );
                        break;
                    case protocol::packet::QoS::RESERVED:
                        rcs.push_back( protocol::suback_return_code::FAILURE );
                        break;
                    default:
                        rcs.push_back( protocol::suback_return_code::FAILURE );
                        break;
                }
            }

            return std::make_shared<const suback>( packet_identifier_, rcs );
        }

        /// \brief Return a string representation to be used in log output.
        ///
        /// \return A string representation to be used in log output
        [[nodiscard]] auto to_string( ) const -> const std::string override
        {
            std::ostringstream output;
            output << "subscribe[pktid:" << packet_identifier_ << "|";
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

}  // namespace io_wally::protocol
