#include "catch.hpp"

#include <cstdint>
#include <array>

#include "io_wally/error/protocol.hpp"
#include "io_wally/codec/pingreq_packet_decoder.hpp"

using namespace io_wally;

SCENARIO( "pingreq_packet_decoder", "[decoder]" )
{
    decoder::pingreq_packet_decoder<const std::uint8_t*> under_test;

    GIVEN( "a well-formed and complete header byte array with 1 length byte" )
    {
        const std::uint8_t type_and_flags = ( 12 << 4 );  // PINGREQ
        const std::uint32_t remaining_length = 0;

        const struct protocol::packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/pingreq_test.go#L132
        const std::array<std::uint8_t, remaining_length> buffer = {{}};  /// avoids warning

        WHEN( "a client passes that array into pingreq_packet_decoder::decode" )
        {
            std::unique_ptr<const protocol::mqtt_packet> result =
                under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'pingreq' instance with all fields correctly set" )
            {
                const protocol::mqtt_packet& raw_result = *result;
                const protocol::pingreq& pingreq_packet = static_cast<const protocol::pingreq&>( raw_result );

                CHECK( pingreq_packet.header( ).type( ) == protocol::packet::Type::PINGREQ );
            }
        }
    }

    GIVEN( "a malformed header byte array with non-zero remaining length" )
    {
        const std::uint8_t type_and_flags = ( 12 << 4 );  // PINGREQ
        const std::uint32_t remaining_length = 1;

        const struct protocol::packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/pingreq_test.go#L132
        const std::array<std::uint8_t, remaining_length> buffer = {{
            0,
        }};  /// avoids warning

        WHEN( "a client passes that array into pingreq_packet_decoder::decode" )
        {

            THEN( "that client should see a error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a malformed header byte array with incorrect remaining length" )
    {
        const std::uint8_t type_and_flags = ( 12 << 4 );  // PINGREQ
        const std::uint32_t remaining_length = 0;

        const struct protocol::packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/pingreq_test.go#L132
        const std::array<std::uint8_t, 1> buffer = {{
            0,
        }};  /// avoids warning

        WHEN( "a client passes that array into pingreq_packet_decoder::decode" )
        {

            THEN( "that client should see a error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }
}
