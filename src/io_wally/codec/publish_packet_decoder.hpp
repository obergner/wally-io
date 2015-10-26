#pragma once

#include <cassert>
#include <cstdint>
#include <sstream>
#include <memory>
#include <vector>
#include <tuple>

#include "io_wally/protocol/subscription.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/codec/decoder.hpp"

namespace io_wally
{
    namespace decoder
    {
        /// \brief \c packet_body_decoder implementation for PUBLISH packets.
        ///
        /// Interprets the supplied buffer to contain a serialized PUBLISH packet. Decodes the buffer and returns
        /// decoded \c publish_packet.
        ///
        /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718nullptr28
        template <typename InputIterator>
        class publish_packet_decoder final : public packet_body_decoder<InputIterator>
        {
           public:
            /// \brief Decode the supplied buffer into a \c publish packet.
            ///
            /// \see io_wally::protocol::decoder::packet_body_decoder::parse
            ///
            virtual std::shared_ptr<const protocol::mqtt_packet> decode( const protocol::packet::header& header,
                                                                         InputIterator buf_start,
                                                                         const InputIterator buf_end ) const
            {
                using namespace io_wally::protocol;

                assert( header.type( ) == packet::Type::PUBLISH );

                // Check that size of supplied buffer corresponds to remaining length as advertised in header
                if ( ( buf_end - buf_start ) != header.remaining_length( ) )
                {
                    std::ostringstream message;
                    message << "Size of supplied buffer does not correspond to "
                            << "remaining length as advertised in header: [expected:" << header.remaining_length( )
                            << "|actual:" << ( buf_end - buf_start ) << "]";
                    throw error::malformed_mqtt_packet( message.str( ) );
                }
                InputIterator application_message_end = buf_start + header.remaining_length( );

                InputIterator new_buf_start = buf_start;

                // Parse topic_name
                std::string topic_name;
                std::tie( new_buf_start, topic_name ) = decode_utf8_string( new_buf_start, buf_end );
                check_well_formed_topic_name( topic_name );

                // Parse variable header publish_header IFF QoS > 0
                uint16_t packet_identifier = -1;
                if ( ( header.flags( ).qos( ) == packet::QoS::AT_LEAST_ONCE ) ||
                     ( header.flags( ).qos( ) == packet::QoS::EXACTLY_ONCE ) )
                {
                    std::tie( new_buf_start, packet_identifier ) = decode_uint16( new_buf_start, buf_end );
                }

                // Parse application message
                std::vector<uint8_t> application_message{new_buf_start, application_message_end};
                new_buf_start = application_message_end;

                return std::make_shared<const protocol::publish>(
                    header, topic_name, packet_identifier, application_message );
            }

           private:
            void check_well_formed_topic_name( const std::string& topic_name ) const
            {
                if ( topic_name.empty( ) )
                    throw error::malformed_mqtt_packet(
                        "[MQTT-4.7.3-1] Topic name MUST be at least one character long" );

                if ( topic_name.size( ) > 65535 )
                    throw error::malformed_mqtt_packet(
                        "[MQTT-4.7.3-3] Topic name MUST NOT be longer than 65535 characters" );

                std::size_t found = topic_name.find( '\0' );
                if ( found != std::string::npos )
                    throw error::malformed_mqtt_packet(
                        "[MQTT-4.7.3-2] Topic name MUST NOT include null character (U+0000)" );

                found = topic_name.find( '+' );
                if ( found != std::string::npos )
                    throw error::malformed_mqtt_packet(
                        "[MQTT-4.7.1-1] Topic name MUST NOT include wildcard character '+'" );

                found = topic_name.find( '#' );
                if ( found != std::string::npos )
                    throw error::malformed_mqtt_packet(
                        "[MQTT-4.7.1-1] Topic name MUST NOT include wildcard character '#'" );
            }
        };  // class publish_packet_decoder

    }  // namespace decoder
}  // namespace io_wally
