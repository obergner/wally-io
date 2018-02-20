#include "catch.hpp"

#include <cstdint>
#include <vector>

#include "io_wally/codec/pingreq_packet_decoder.hpp"
#include "io_wally/error/protocol.hpp"

using namespace io_wally;

SCENARIO( "pingreq_packet_decoder_impl", "[decoder]" )
{
    auto under_test = decoder::pingreq_packet_decoder_impl{};

    GIVEN( "a well-formed and complete header byte array with 1 length byte" )
    {
        const auto type_and_flags = std::uint8_t{12 << 4};  // PINGREQ
        const auto buffer = std::vector<std::uint8_t>{};
        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that array into pingreq_packet_decoder::decode" )
        {
            std::shared_ptr<const protocol::mqtt_packet> result = under_test.decode( frame );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'pingreq' instance with all fields correctly set" )
            {
                const protocol::mqtt_packet& raw_result = *result;
                const protocol::pingreq& pingreq_packet = static_cast<const protocol::pingreq&>( raw_result );

                CHECK( pingreq_packet.type( ) == protocol::packet::Type::PINGREQ );
            }
        }
    }

    GIVEN( "a malformed header byte array with non-zero remaining length" )
    {
        const auto type_and_flags = std::uint8_t{12 << 4};  // PINGREQ
        const auto buffer = std::vector<std::uint8_t>{
            0x00,
            0x01,
        };
        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that array into pingreq_packet_decoder::decode" )
        {

            THEN( "that client should see a error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( frame ), error::malformed_mqtt_packet );
            }
        }
    }
}
