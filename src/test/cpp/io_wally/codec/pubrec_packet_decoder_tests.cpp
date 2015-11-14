#include "catch.hpp"

#include <cstdint>
#include <array>
#include <memory>

#include "io_wally/error/protocol.hpp"
#include "io_wally/codec/pubrec_packet_decoder.hpp"

using namespace io_wally;

SCENARIO( "pubrec_packet_decoder", "[decoder]" )
{
    decoder::pubrec_packet_decoder<const std::uint8_t*> under_test{};

    GIVEN( "a well-formed PUBREC" )
    {
        const std::uint8_t type_and_flags = ( 5 << 4 ) | 0;  // PUBREC
        const std::uint32_t remaining_length = 2;

        const struct protocol::packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/pubrec_test.go#L132
        const std::array<std::uint8_t, remaining_length> buffer = {{
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
        }};     /// avoids warning

        WHEN( "a client passes that array into pubrec_packet_decoder::decode" )
        {
            std::shared_ptr<const protocol::mqtt_packet> result =
                under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'pubrec' instance with all fields correctly set" )
            {
                const protocol::mqtt_packet& raw_result = *result;
                const protocol::pubrec& pubrec_packet = static_cast<const protocol::pubrec&>( raw_result );

                CHECK( pubrec_packet.header( ).type( ) == protocol::packet::Type::PUBREC );

                CHECK( pubrec_packet.packet_identifier( ) == 7 );
            }
        }
    }

    GIVEN( "a PUBREC packet with a header having illegal flags set" )
    {
        const std::uint8_t type_and_flags = ( 5 << 4 ) | 1;  // PUBREC
        const std::uint32_t remaining_length = 2;

        const struct protocol::packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/pubrec_test.go#L132
        const std::array<std::uint8_t, remaining_length> buffer = {{
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
        }};     /// avoids warning

        WHEN( "a client passes that header into pubrec_packet_decoder::decode" )
        {
            THEN( "that client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }
}