#include "catch.hpp"

#include <cstdint>
#include <array>
#include <memory>

#include "io_wally/error/protocol.hpp"
#include "io_wally/codec/pubcomp_packet_decoder.hpp"

using namespace io_wally;

SCENARIO( "pubcomp_packet_decoder", "[decoder]" )
{
    decoder::pubcomp_packet_decoder<const std::uint8_t*> under_test{};

    GIVEN( "a well-formed PUBCOMP" )
    {
        const std::uint8_t type_and_flags = ( 7 << 4 ) | 0;  // PUBCOMP
        const std::uint32_t remaining_length = 2;

        const struct protocol::packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/pubcomp_test.go#L132
        const std::array<std::uint8_t, remaining_length> buffer = {{
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
        }};     /// avoids warning

        WHEN( "a client passes that array into pubcomp_packet_decoder::decode" )
        {
            std::shared_ptr<const protocol::mqtt_packet> result =
                under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'pubcomp' instance with all fields correctly set" )
            {
                const protocol::mqtt_packet& raw_result = *result;
                const protocol::pubcomp& pubcomp_packet = static_cast<const protocol::pubcomp&>( raw_result );

                CHECK( pubcomp_packet.header( ).type( ) == protocol::packet::Type::PUBCOMP );

                CHECK( pubcomp_packet.packet_identifier( ) == 7 );
            }
        }
    }

    GIVEN( "a PUBCOMP packet with a header having illegal flags set" )
    {
        const std::uint8_t type_and_flags = ( 7 << 4 ) | 4;  // PUBCOMP
        const std::uint32_t remaining_length = 2;

        const struct protocol::packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/pubcomp_test.go#L132
        const std::array<std::uint8_t, remaining_length> buffer = {{
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
        }};     /// avoids warning

        WHEN( "a client passes that header into pubcomp_packet_decoder::decode" )
        {
            THEN( "that client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }
}
