#pragma once

#include <cassert>
#include <sstream>
#include <memory>

#include "io_wally/protocol/disconnect_packet.hpp"
#include "io_wally/codec/decoder.hpp"

namespace io_wally
{
    namespace decoder
    {
        /// \brief \c packet_body_decoder implementation for DISCONNECT packets.
        ///
        /// Interprets the supplied buffer to contain a serialized DISCONNECT packet. Decodes the buffer and returns
        /// decoded \c disconnect_packet.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718081
        template <typename InputIterator>
        class disconnect_packet_decoder final : public packet_body_decoder<InputIterator>
        {
           public:
            /// \brief Decode the supplied buffer into a \c disconnect_packet.
            ///
            /// \see io_wally::protocol::decoder::packet_body_decoder::decode
            ///
            virtual std::unique_ptr<const protocol::mqtt_packet> decode( const protocol::packet::header& header,
                                                                         InputIterator buf_start,
                                                                         const InputIterator buf_end ) const
            {
                using namespace io_wally::protocol;

                // TODO:: consider removing this assert in release build
                assert( header.type( ) == packet::Type::DISCONNECT );

                // MQTT-3.14.1-1: Header flags MUST be zero
                if ( ( header.type_and_flags( ) & 0x0F ) != 0 )
                {
                    std::ostringstream message;
                    message << "DISCONNECT header has non-zero flags set (violates MQTT 3.1.1 spec)";
                    throw error::malformed_mqtt_packet( message.str( ) );
                }

                // Check that remaining length is 0, as required by MQTT 3.1.1
                if ( header.remaining_length( ) != 0 )
                {
                    std::ostringstream message;
                    message << "DISCONNECT fixed header reports remaining length != 0 (violates MQTT 3.1.1 spec)";
                    throw error::malformed_mqtt_packet( message.str( ) );
                }

                // Check that size of supplied buffer corresponds to remaining length as advertised in header
                if ( ( buf_end - buf_start ) != header.remaining_length( ) )
                {
                    std::ostringstream message;
                    message << "Size of supplied buffer does not correspond to "
                            << "remaining length as advertised in header: [expected:" << header.remaining_length( )
                            << "|actual:" << ( buf_end - buf_start ) << "]";
                    throw error::malformed_mqtt_packet( message.str( ) );
                }

                return std::make_unique<const protocol::disconnect>( );
            }
        };
    }  // namespace decoder
}  // namespace io_wally
