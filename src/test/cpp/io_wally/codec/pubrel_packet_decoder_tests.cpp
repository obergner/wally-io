#include "catch.hpp"

#include <cstdint>
#include <array>
#include <memory>

#include "io_wally/error/protocol.hpp"
#include "io_wally/codec/pubrel_packet_decoder.hpp"

using namespace io_wally;

SCENARIO( "pubrel_packet_decoder", "[decoder]" )
{
    decoder::pubrel_packet_decoder<const std::uint8_t*> under_test{};

    GIVEN( "a well-formed PUBREL" )
    {
        const std::uint8_t type_and_flags = ( 6 << 4 ) | 2;  // PUBREL
        const std::uint32_t remaining_length = 2;

        const struct protocol::packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/pubrel_test.go#L132
        const std::array<std::uint8_t, remaining_length> buffer = {{
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
        }};     /// avoids warning

        WHEN( "a client passes that array into pubrel_packet_decoder::decode" )
        {
            std::shared_ptr<const protocol::mqtt_packet> result =
                under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'pubrel' instance with all fields correctly set" )
            {
                const protocol::mqtt_packet& raw_result = *result;
                const protocol::pubrel& pubrel_packet = static_cast<const protocol::pubrel&>( raw_result );

                CHECK( pubrel_packet.header( ).type( ) == protocol::packet::Type::PUBREL );

                CHECK( pubrel_packet.packet_identifier( ) == 7 );
            }
        }
    }

    GIVEN( "a PUBREL packet with a header having illegal flags set" )
    {
        const std::uint8_t type_and_flags = ( 6 << 4 ) | 1;  // PUBREL
        const std::uint32_t remaining_length = 2;

        const struct protocol::packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/pubrel_test.go#L132
        const std::array<std::uint8_t, remaining_length> buffer = {{
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
        }};     /// avoids warning

        WHEN( "a client passes that header into pubrel_packet_decoder::decode" )
        {
            THEN( "that client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }
}

SCENARIO( "pubrel_packet_decoder_impl", "[decoder]" )
{
    const auto under_test = decoder::pubrel_packet_decoder_impl{};

    GIVEN( "a well-formed PUBREL" )
    {
        const auto type_and_flags = std::uint8_t{( 6 << 4 ) | 2};  // PUBREL
        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/pubrel_test.go#L132
        const std::vector<std::uint8_t> buffer = {
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
        };      /// avoids warning
        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that array into pubrel_packet_decoder::decode" )
        {
            std::shared_ptr<const protocol::mqtt_packet> result = under_test.decode( frame );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'pubrel' instance with all fields correctly set" )
            {
                const protocol::mqtt_packet& raw_result = *result;
                const protocol::pubrel& pubrel_packet = static_cast<const protocol::pubrel&>( raw_result );

                CHECK( pubrel_packet.header( ).type( ) == protocol::packet::Type::PUBREL );

                CHECK( pubrel_packet.packet_identifier( ) == 7 );
            }
        }
    }

    GIVEN( "a PUBREL packet with a header having illegal flags set" )
    {
        const auto type_and_flags = std::uint8_t{( 6 << 4 ) | 1};  // PUBREL
        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/pubrel_test.go#L132
        const std::vector<std::uint8_t> buffer = {
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
        };      /// avoids warning
        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that header into pubrel_packet_decoder::decode" )
        {
            THEN( "that client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( frame ), error::malformed_mqtt_packet );
            }
        }
    }
}
