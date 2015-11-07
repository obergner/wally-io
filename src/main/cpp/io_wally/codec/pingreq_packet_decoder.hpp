#pragma once

#include <cassert>
#include <memory>
#include <sstream>

#include "io_wally/protocol/pingreq_packet.hpp"
#include "io_wally/codec/decoder.hpp"

namespace io_wally
{
    namespace decoder
    {
        /// \brief \c packet_body_decoder implementation for PINGREQ packets.
        ///
        /// Interprets the supplied buffer to contain a serialized PINGREQ packet. Decodes the buffer and returns
        /// decoded \c pingreq_packet.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718081
        template <typename InputIterator>
        class pingreq_packet_decoder final : public packet_body_decoder<InputIterator>
        {
           public:
            /// \brief Decode the supplied buffer into a \c pingreq_packet.
            ///
            /// \see io_wally::protocol::decoder::packet_body_decoder::decode
            ///
            virtual std::shared_ptr<protocol::mqtt_packet> decode( const protocol::packet::header& header,
                                                                   InputIterator buf_start,
                                                                   const InputIterator buf_end ) const
            {
                // TODO:: consider removing this assert in release build
                assert( header.type( ) == protocol::packet::Type::PINGREQ );

                // Check that remaining length is 0, as required by MQTT 3.1.1
                if ( header.remaining_length( ) != 0 )
                {
                    std::ostringstream message;
                    message << "PINGREQ fixed header reports remaining length != 0 (violates MQTT 3.1.1 spec)";
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

                return std::make_shared<protocol::pingreq>( );
            }
        };
    }  // namespace decoder
}  // namespace io_wally
