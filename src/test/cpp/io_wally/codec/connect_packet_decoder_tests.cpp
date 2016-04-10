#include "catch.hpp"

#include <cstdint>
#include <vector>
#include <memory>

#include "io_wally/error/protocol.hpp"
#include "io_wally/codec/connect_packet_decoder.hpp"

using namespace io_wally;

SCENARIO( "connect_packet_decoder_impl", "[decoder]" )
{
    auto under_test = decoder::connect_packet_decoder_impl{};

    GIVEN( "a well-formed and complete header byte array with 1 length byte" )
    {
        const auto type_and_flags = std::uint8_t{1 << 4};  // CONNECT

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/connect_test.go#L132
        const std::vector<std::uint8_t> buffer = {{
            0,  // Length MSB (0)
            4,  // Length LSB (4)
            'M', 'Q', 'T', 'T',
            4,    // Protocol level 4
            206,  // connect flags 11001110, will QoS = 01
            0,    // Keep Alive MSB (0)
            10,   // Keep Alive LSB (10)
            0,    // Client ID MSB (0)
            7,    // Client ID LSB (7)
            's', 'u', 'r', 'g', 'e', 'm', 'q',
            0,  // Will Topic MSB (0)
            4,  // Will Topic LSB (4)
            'w', 'i', 'l', 'l',
            0,   // Will Message MSB (0)
            12,  // Will Message LSB (12)
            's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e',
            0,  // Username ID MSB (0)
            7,  // Username ID LSB (7)
            's', 'u', 'r', 'g', 'e', 'm', 'q',
            0,   // Password ID MSB (0)
            10,  // Password ID LSB (10)
            'v', 'e', 'r', 'y', 's', 'e', 'c', 'r', 'e', 't',
        }};  /// avoids warning

        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that array into connect_packet_decoder::decode" )
        {
            std::shared_ptr<const protocol::mqtt_packet> result = under_test.decode( frame );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'connect' instance with all fields correctly set" )
            {
                const protocol::mqtt_packet& raw_result = *result;
                const protocol::connect& connect_packet = static_cast<const protocol::connect&>( raw_result );

                CHECK( connect_packet.header( ).type( ) == protocol::packet::Type::CONNECT );

                CHECK( connect_packet.contains_last_will( ) );
                CHECK( connect_packet.last_will_qos( ) == protocol::packet::QoS::AT_LEAST_ONCE );
                CHECK( !connect_packet.retain_last_will( ) );
                CHECK( connect_packet.protocol_level( ) == protocol::packet::ProtocolLevel::LEVEL4 );
                CHECK( connect_packet.protocol_name( ) == "MQTT" );
                CHECK( connect_packet.clean_session( ) );
                CHECK( connect_packet.has_username( ) );
                CHECK( connect_packet.has_password( ) );
                CHECK( connect_packet.has_password( ) );
                CHECK( connect_packet.keep_alive_secs( ) == 10 );

                CHECK( connect_packet.client_id( ) == "surgemq" );
                CHECK( connect_packet.will_topic( ) );
                CHECK( *connect_packet.will_topic( ) == "will" );
                CHECK( connect_packet.will_message( ) );
                CHECK( *connect_packet.will_message( ) == "send me home" );
                CHECK( connect_packet.username( ) );
                CHECK( *connect_packet.username( ) == "surgemq" );
                CHECK( connect_packet.password( ) );
                CHECK( *connect_packet.password( ) == "verysecret" );
            }
        }
    }

    GIVEN( "a malformed header byte array with incorrectly encoded will topic" )
    {
        const auto type_and_flags = std::uint8_t{1 << 4};  // CONNECT

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/connect_test.go#L132
        const std::vector<std::uint8_t> buffer = {{
            0,  // Length MSB (0)
            4,  // Length LSB (4)
            'M', 'Q', 'T', 'T',
            4,    // Protocol level 4
            206,  // connect flags 11001110, will QoS = 01
            0,    // Keep Alive MSB (0)
            10,   // Keep Alive LSB (10)
            0,    // Client ID MSB (0)
            7,    // Client ID LSB (7)
            's', 'u', 'r', 'g', 'e', 'm', 'q',
            0,  // Will Topic MSB (0)
            5,  // Will Topic LSB (5) !WRONG!
            'w', 'i', 'l', 'l',
            0,   // Will Message MSB (0)
            12,  // Will Message LSB (12)
            's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e',
            0,  // Username ID MSB (0)
            7,  // Username ID LSB (7)
            's', 'u', 'r', 'g', 'e', 'm', 'q',
            0,   // Password ID MSB (0)
            10,  // Password ID LSB (10)
            'v', 'e', 'r', 'y', 's', 'e', 'c', 'r', 'e', 't',
        }};  /// avoids warning

        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that array into connect_packet_decoder::decode" )
        {

            THEN( "that client should see a error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( frame ), error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a malformed header byte array with advertised but missing username" )
    {
        const auto type_and_flags = std::uint8_t{1 << 4};  // CONNECT

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/connect_test.go#L132
        const std::vector<std::uint8_t> buffer = {{
            0,  // Length MSB (0)
            4,  // Length LSB (4)
            'M', 'Q', 'T', 'T',
            4,    // Protocol level 4
            206,  // connect flags 11001110, will QoS = 01
            0,    // Keep Alive MSB (0)
            10,   // Keep Alive LSB (10)
            0,    // Client ID MSB (0)
            7,    // Client ID LSB (7)
            's', 'u', 'r', 'g', 'e', 'm', 'q',
            0,  // Will Topic MSB (0)
            4,  // Will Topic LSB (4)
            'w', 'i', 'l', 'l',
            0,   // Will Message MSB (0)
            21,  // Will Message LSB (21)
            's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x',
            0,   // Password ID MSB (0)
            10,  // Password ID LSB (10)
            'v', 'e', 'r', 'y', 's', 'e', 'c', 'r', 'e', 't',
        }};  /// avoids warning

        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that array into connect_packet_decoder::decode" )
        {

            THEN( "that client should see a error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( frame ), error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a malformed header byte array with field lengths that don't add up" )
    {
        const auto type_and_flags = std::uint8_t{1 << 4};  // CONNECT

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/connect_test.go#L132
        const std::vector<std::uint8_t> buffer = {{
            0,  // Length MSB (0)
            4,  // Length LSB (4)
            'M', 'Q', 'T', 'T',
            4,    // Protocol level 4
            206,  // connect flags 11001110, will QoS = 01
            0,    // Keep Alive MSB (0)
            10,   // Keep Alive LSB (10)
            0,    // Client ID MSB (0)
            7,    // Client ID LSB (7)
            's', 'u', 'r', 'g', 'e', 'm', 'q',
            0,  // Will Topic MSB (0)
            4,  // Will Topic LSB (4)
            'w', 'i', 'l', 'l',
            0,   // Will Message MSB (0)
            12,  // Will Message LSB (12)
            's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e',
            0,  // Username ID MSB (0)
            7,  // Username ID LSB (7)
            's', 'u', 'r', 'g', 'e', 'm', 'q',
            0,  // Password ID MSB (0)
            9,  // Password ID LSB (9) !ONE BYTE SHORT!
            'v', 'e', 'r', 'y', 's', 'e', 'c', 'r', 'e', 'x',
        }};  /// avoids warning

        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that array into connect_packet_decoder::decode" )
        {

            THEN( "that client should see a error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( frame ), error::malformed_mqtt_packet );
            }
        }
    }
}
