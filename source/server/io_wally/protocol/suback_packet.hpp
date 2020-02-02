#pragma once

#include <cstdint>
#include <sstream>
#include <vector>

#include "io_wally/protocol/common.hpp"

namespace io_wally::protocol
{
    /// \brief Allowd SUBACK return codes as defined by MQTT 3.1.1
    ///
    /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718068
    enum class suback_return_code : uint8_t
    {
        /// Success - maximum QoS 0
        MAXIMUM_QOS0 = 0x00,

        /// Success - maximum QoS 1
        MAXIMUM_QOS1,

        /// Success - maximum QoS 2
        MAXIMUM_QOS2,

        /// Topic subscription failed
        FAILURE = 0x80,

        /// Reserved for future use
        RESERVED
    };

    /// \brief Overload stream output operator for \c suback_return_code.
    ///
    /// Overload stream output operator for \c suback_return_code, primarily to facilitate logging.
    inline auto operator<<( std::ostream& output, suback_return_code const& return_code ) -> std::ostream&
    {
        switch ( return_code )
        {
            case suback_return_code::MAXIMUM_QOS0:
                output << "Success: maximum QoS 0";
                break;
            case suback_return_code::MAXIMUM_QOS1:
                output << "Success: maximum QoS 1";
                break;
            case suback_return_code::MAXIMUM_QOS2:
                output << "Success: maximum QoS 2";
                break;
            case suback_return_code::FAILURE:
                output << "Failure";
                break;
            case suback_return_code::RESERVED:
                output << "Reserved for future use";
                break;
            default:
                assert( false );
                break;
        }

        return output;
    }

    /// \brief SUBACK packet, sent in response to a \c suback packet.
    ///
    /// A SUBACK packet contains in its variable header the \c packet_identifier sent by a client in the
    /// corresponding SUBSCRIBE packet.
    ///
    /// The SUBACK packet's payload contains a list of return codes, corresponding to the list of topic
    /// subscriptions in the acknowledged SUBSCRIBE packet, in that order.
    ///
    /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718068
    struct suback final : public mqtt_ack
    {
       public:
        suback( const uint16_t packet_identifier, std::vector<suback_return_code> return_codes )
            : mqtt_ack{packet::Type::SUBACK, 2 + static_cast<uint32_t>( return_codes.size( ) )},
              packet_identifier_{packet_identifier},
              return_codes_{std::move( return_codes )}
        {
        }

        /// \brief Return \c packet_identifier sent in corresponding SUBSCRIBE packet.
        ///
        /// \return \c packet_identifier sent in corresponding SUBSCRIBE packet.
        [[nodiscard]] auto packet_identifier( ) const -> uint16_t
        {
            return packet_identifier_;
        }

        /// \brief The list of return codes, each return code corresponding to a \c topic \c filter received in
        /// corresponding SUBSCRIBE packet. Order of return codes MUST conform to order of corresponding \c topic
        /// filters.
        ///
        /// \return List of \c suback_return_codes, corresponding to \c topic \c filters in corresponding SUBSCRIBE
        /// packet.
        [[nodiscard]] auto return_codes( ) const -> const std::vector<suback_return_code>&
        {
            return return_codes_;
        }

        /// \return A string representation to be used in log output
        [[nodiscard]] auto to_string( ) const -> const std::string override
        {
            auto output = std::ostringstream{};
            output << "suback[pktid:" << packet_identifier_ << "|rcs:";
            for ( auto& src : return_codes_ )
                output << src << ":";
            const long pos = output.tellp( );
            output.seekp( pos - 1 );
            output << "]";

            return output.str( );
        }

       private:
        const uint16_t packet_identifier_;
        const std::vector<suback_return_code> return_codes_;
    };  // struct suback

}  // namespace io_wally::protocol
