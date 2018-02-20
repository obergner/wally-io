#include "catch.hpp"

#include <cstdint>
#include <memory>
#include <vector>

#include "io_wally/codec/puback_packet_decoder.hpp"
#include "io_wally/error/protocol.hpp"

using namespace io_wally;

SCENARIO( "puback_packet_decoder_impl", "[decoder]" )
{
    auto under_test = decoder::puback_packet_decoder_impl{};

    GIVEN( "a well-formed PUBACK" )
    {
        const auto type_and_flags = std::uint8_t{( 4 << 4 ) | 0};  // PUBACK
        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/puback_test.go#L132
        const std::vector<std::uint8_t> buffer = {
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
        };
        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that array into puback_packet_decoder::decode" )
        {
            std::shared_ptr<const protocol::mqtt_packet> result = under_test.decode( frame );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'puback' instance with all fields correctly set" )
            {
                const protocol::mqtt_packet& raw_result = *result;
                const protocol::puback& puback_packet = static_cast<const protocol::puback&>( raw_result );

                CHECK( puback_packet.type( ) == protocol::packet::Type::PUBACK );

                CHECK( puback_packet.packet_identifier( ) == 7 );
            }
        }
    }

    GIVEN( "a PUBACK packet with a header having illegal flags set" )
    {
        const auto type_and_flags = std::uint8_t{( 4 << 4 ) | 1};  // PUBACK
        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/puback_test.go#L132
        const std::vector<std::uint8_t> buffer = {
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
        };
        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that header into puback_packet_decoder::decode" )
        {
            THEN( "that client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( frame ), error::malformed_mqtt_packet );
            }
        }
    }
}
