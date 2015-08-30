#include "catch.hpp"

#include "io_wally/codec/connect_packet_decoder.hpp"

using namespace io_wally;

SCENARIO( "connect_packet_decoder", "[decoder]" )
{
    decoder::connect_packet_decoder<const uint8_t*> under_test;

    GIVEN( "a well-formed and complete header byte array with 1 length byte" )
    {
        const uint8_t type_and_flags = ( 1 << 4 );  // CONNECT
        const uint32_t remaining_length = 60;

        const struct packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/connect_test.go#L132
        const std::array<uint8_t, remaining_length> buffer = {{
            0,  // Length MSB (0)
            4,  // Length LSB (4)
            'M',
            'Q',
            'T',
            'T',
            4,    // Protocol level 4
            206,  // connect flags 11001110, will QoS = 01
            0,    // Keep Alive MSB (0)
            10,   // Keep Alive LSB (10)
            0,    // Client ID MSB (0)
            7,    // Client ID LSB (7)
            's',
            'u',
            'r',
            'g',
            'e',
            'm',
            'q',
            0,  // Will Topic MSB (0)
            4,  // Will Topic LSB (4)
            'w',
            'i',
            'l',
            'l',
            0,   // Will Message MSB (0)
            12,  // Will Message LSB (12)
            's',
            'e',
            'n',
            'd',
            ' ',
            'm',
            'e',
            ' ',
            'h',
            'o',
            'm',
            'e',
            0,  // Username ID MSB (0)
            7,  // Username ID LSB (7)
            's',
            'u',
            'r',
            'g',
            'e',
            'm',
            'q',
            0,   // Password ID MSB (0)
            10,  // Password ID LSB (10)
            'v',
            'e',
            'r',
            'y',
            's',
            'e',
            'c',
            'r',
            'e',
            't', }};  /// avoids warning

        WHEN( "a client passes that array into connect_packet_decoder::decode" )
        {
            std::unique_ptr<const mqtt_packet> result =
                under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'connect' instance with all fields correctly set" )
            {
                const mqtt_packet& raw_result = *result;
                const connect& connect_packet = static_cast<const connect&>( raw_result );

                CHECK( connect_packet.header( ).type( ) == packet::Type::CONNECT );

                CHECK( connect_packet.connect_header( ).contains_last_will( ) );
                CHECK( connect_packet.connect_header( ).last_will_qos( ) == packet::QoS::AT_LEAST_ONCE );
                CHECK( !connect_packet.connect_header( ).retain_last_will( ) );
                CHECK( connect_packet.connect_header( ).protocol_level( ) == packet::ProtocolLevel::LEVEL4 );
                CHECK( connect_packet.connect_header( ).protocol_name( ) == "MQTT" );
                CHECK( connect_packet.connect_header( ).clean_session( ) );
                CHECK( connect_packet.connect_header( ).has_username( ) );
                CHECK( connect_packet.connect_header( ).has_password( ) );
                CHECK( connect_packet.connect_header( ).has_password( ) );
                CHECK( connect_packet.connect_header( ).keep_alive_secs( ) == 10 );

                CHECK( connect_packet.payload( ).client_id( ) == "surgemq" );
                CHECK( connect_packet.payload( ).will_topic( ) );
                CHECK( *connect_packet.payload( ).will_topic( ) == "will" );
                CHECK( connect_packet.payload( ).will_message( ) );
                CHECK( *connect_packet.payload( ).will_message( ) == "send me home" );
                CHECK( connect_packet.payload( ).username( ) );
                CHECK( *connect_packet.payload( ).username( ) == "surgemq" );
                CHECK( connect_packet.payload( ).password( ) );
                CHECK( *connect_packet.payload( ).password( ) == "verysecret" );
            }
        }
    }

    GIVEN( "a malformed header byte array with incorrect remaining length" )
    {
        const uint8_t type_and_flags = ( 1 << 4 );  // CONNECT
        const uint32_t remaining_length = 59;

        const struct packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/connect_test.go#L132
        const std::array<uint8_t, 60> buffer = {{
            0,  // Length MSB (0)
            4,  // Length LSB (4)
            'M',
            'Q',
            'T',
            'T',
            4,    // Protocol level 4
            206,  // connect flags 11001110, will QoS = 01
            0,    // Keep Alive MSB (0)
            10,   // Keep Alive LSB (10)
            0,    // Client ID MSB (0)
            7,    // Client ID LSB (7)
            's',
            'u',
            'r',
            'g',
            'e',
            'm',
            'q',
            0,  // Will Topic MSB (0)
            4,  // Will Topic LSB (4)
            'w',
            'i',
            'l',
            'l',
            0,   // Will Message MSB (0)
            12,  // Will Message LSB (12)
            's',
            'e',
            'n',
            'd',
            ' ',
            'm',
            'e',
            ' ',
            'h',
            'o',
            'm',
            'e',
            0,  // Username ID MSB (0)
            7,  // Username ID LSB (7)
            's',
            'u',
            'r',
            'g',
            'e',
            'm',
            'q',
            0,   // Password ID MSB (0)
            10,  // Password ID LSB (10)
            'v',
            'e',
            'r',
            'y',
            's',
            'e',
            'c',
            'r',
            'e',
            't', }};  /// avoids warning

        WHEN( "a client passes that array into connect_packet_decoder::decode" )
        {

            THEN( "that client should see a decoder::error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) ),
                                   decoder::error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a malformed header byte array with incorrectly encoded will topic" )
    {
        const uint8_t type_and_flags = ( 1 << 4 );  // CONNECT
        const uint32_t remaining_length = 60;

        const struct packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/connect_test.go#L132
        const std::array<uint8_t, remaining_length> buffer = {{
            0,  // Length MSB (0)
            4,  // Length LSB (4)
            'M',
            'Q',
            'T',
            'T',
            4,    // Protocol level 4
            206,  // connect flags 11001110, will QoS = 01
            0,    // Keep Alive MSB (0)
            10,   // Keep Alive LSB (10)
            0,    // Client ID MSB (0)
            7,    // Client ID LSB (7)
            's',
            'u',
            'r',
            'g',
            'e',
            'm',
            'q',
            0,  // Will Topic MSB (0)
            5,  // Will Topic LSB (5) !WRONG!
            'w',
            'i',
            'l',
            'l',
            0,   // Will Message MSB (0)
            12,  // Will Message LSB (12)
            's',
            'e',
            'n',
            'd',
            ' ',
            'm',
            'e',
            ' ',
            'h',
            'o',
            'm',
            'e',
            0,  // Username ID MSB (0)
            7,  // Username ID LSB (7)
            's',
            'u',
            'r',
            'g',
            'e',
            'm',
            'q',
            0,   // Password ID MSB (0)
            10,  // Password ID LSB (10)
            'v',
            'e',
            'r',
            'y',
            's',
            'e',
            'c',
            'r',
            'e',
            't', }};  /// avoids warning

        WHEN( "a client passes that array into connect_packet_decoder::decode" )
        {

            THEN( "that client should see a decoder::error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) ),
                                   decoder::error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a malformed header byte array with advertised but missing username" )
    {
        const uint8_t type_and_flags = ( 1 << 4 );  // CONNECT
        const uint32_t remaining_length = 60;

        const struct packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/connect_test.go#L132
        const std::array<uint8_t, remaining_length> buffer = {{
            0,  // Length MSB (0)
            4,  // Length LSB (4)
            'M',
            'Q',
            'T',
            'T',
            4,    // Protocol level 4
            206,  // connect flags 11001110, will QoS = 01
            0,    // Keep Alive MSB (0)
            10,   // Keep Alive LSB (10)
            0,    // Client ID MSB (0)
            7,    // Client ID LSB (7)
            's',
            'u',
            'r',
            'g',
            'e',
            'm',
            'q',
            0,  // Will Topic MSB (0)
            4,  // Will Topic LSB (4)
            'w',
            'i',
            'l',
            'l',
            0,   // Will Message MSB (0)
            21,  // Will Message LSB (21)
            's',
            'e',
            'n',
            'd',
            ' ',
            'm',
            'e',
            ' ',
            'h',
            'o',
            'm',
            'e',
            'x',
            'x',
            'x',
            'x',
            'x',
            'x',
            'x',
            'x',
            'x',
            0,   // Password ID MSB (0)
            10,  // Password ID LSB (10)
            'v',
            'e',
            'r',
            'y',
            's',
            'e',
            'c',
            'r',
            'e',
            't', }};  /// avoids warning

        WHEN( "a client passes that array into connect_packet_decoder::decode" )
        {

            THEN( "that client should see a decoder::error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) ),
                                   decoder::error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a malformed header byte array with field lengths that don't add up" )
    {
        const uint8_t type_and_flags = ( 1 << 4 );  // CONNECT
        const uint32_t remaining_length = 60;

        const struct packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/connect_test.go#L132
        const std::array<uint8_t, remaining_length> buffer = {{
            0,  // Length MSB (0)
            4,  // Length LSB (4)
            'M',
            'Q',
            'T',
            'T',
            4,    // Protocol level 4
            206,  // connect flags 11001110, will QoS = 01
            0,    // Keep Alive MSB (0)
            10,   // Keep Alive LSB (10)
            0,    // Client ID MSB (0)
            7,    // Client ID LSB (7)
            's',
            'u',
            'r',
            'g',
            'e',
            'm',
            'q',
            0,  // Will Topic MSB (0)
            4,  // Will Topic LSB (4)
            'w',
            'i',
            'l',
            'l',
            0,   // Will Message MSB (0)
            12,  // Will Message LSB (12)
            's',
            'e',
            'n',
            'd',
            ' ',
            'm',
            'e',
            ' ',
            'h',
            'o',
            'm',
            'e',
            0,  // Username ID MSB (0)
            7,  // Username ID LSB (7)
            's',
            'u',
            'r',
            'g',
            'e',
            'm',
            'q',
            0,  // Password ID MSB (0)
            9,  // Password ID LSB (9) !ONE BYTE SHORT!
            'v',
            'e',
            'r',
            'y',
            's',
            'e',
            'c',
            'r',
            'e',
            'x', }};  /// avoids warning

        WHEN( "a client passes that array into connect_packet_decoder::decode" )
        {

            THEN( "that client should see a decoder::error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) ),
                                   decoder::error::malformed_mqtt_packet );
            }
        }
    }
}
