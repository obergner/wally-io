#include "catch.hpp"

#include <cstdint>
#include <array>

#include "io_wally/codec/mqtt_packet_decoder.hpp"

using namespace io_wally;

SCENARIO( "mqtt_packet_decoder", "[decoder]" )
{
    decoder::mqtt_packet_decoder<const std::uint8_t*> under_test;

    GIVEN( "a well-formed and complete header byte array of type CONNECT with 1 length byte" )
    {
        const std::uint8_t type_and_flags = ( 1 << 4 );  // CONNECT
        const std::uint32_t remaining_length = 60;

        const struct protocol::packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/connect_test.go#L132
        const std::array<std::uint8_t, remaining_length> buffer = {{
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
            't',
        }};  /// avoids warning

        WHEN( "a client passes that array into mqtt_packet_decoder::parse" )
        {
            std::shared_ptr<const protocol::mqtt_packet> result =
                under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) );

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

    GIVEN( "a well-formed and complete header byte array of type PINGREQ with 1 length byte" )
    {
        const std::uint8_t type_and_flags = ( 12 << 4 );  // PINGREQ
        const std::uint32_t remaining_length = 0;

        const struct protocol::packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/connect_test.go#L132
        const std::array<std::uint8_t, remaining_length> buffer = {{}};  /// avoids warning

        WHEN( "a client passes that array into mqtt_packet_decoder::parse" )
        {
            std::shared_ptr<const protocol::mqtt_packet> result =
                under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'pingreq' instance" )
            {
                const protocol::mqtt_packet& raw_result = *result;
                const protocol::pingreq& pingreq_packet = static_cast<const protocol::pingreq&>( raw_result );

                CHECK( pingreq_packet.header( ).type( ) == protocol::packet::Type::PINGREQ );
            }
        }
    }

    GIVEN( "a well-formed and complete SUSBCRIBE packet body containing 3 subscriptions" )
    {
        const std::uint8_t type_and_flags = ( 8 << 4 ) | 2;  // SUBSCRIBE
        const std::uint32_t remaining_length = 36;

        const struct protocol::packet::header fixed_header( type_and_flags, remaining_length );

        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/subscribe_test.go#L132
        const std::array<std::uint8_t, remaining_length> buffer = {{
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
            0,  // topic name MSB (0)
            7,  // topic name LSB (7)
            's',
            'u',
            'r',
            'g',
            'e',
            'm',
            'q',
            0,  // QoS
            0,  // topic name MSB (0)
            8,  // topic name LSB (8)
            '/',
            'a',
            '/',
            'b',
            '/',
            'c',
            '/',
            '#',
            1,   // QoS
            0,   // topic name MSB (0)
            10,  // topic name LSB (10)
            '/',
            'a',
            '/',
            'b',
            '/',
            'c',
            'd',
            'd',
            '/',
            '#',
            2  // QoS
        }};    /// avoids warning

        WHEN( "a client passes that array into subscribe_packet_decoder::decode" )
        {
            std::shared_ptr<const protocol::mqtt_packet> result =
                under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'subscribe' instance with all fields correctly set" )
            {
                const protocol::mqtt_packet& raw_result = *result;
                const protocol::subscribe& subscribe_packet = static_cast<const protocol::subscribe&>( raw_result );

                CHECK( subscribe_packet.header( ).type( ) == protocol::packet::Type::SUBSCRIBE );

                CHECK( subscribe_packet.packet_identifier( ) == 7 );

                CHECK( subscribe_packet.subscriptions( ).size( ) == 3 );

                CHECK( subscribe_packet.subscriptions( )[0].topic_filter( ) == "surgemq" );
                CHECK( subscribe_packet.subscriptions( )[0].maximum_qos( ) == protocol::packet::QoS::AT_MOST_ONCE );

                CHECK( subscribe_packet.subscriptions( )[1].topic_filter( ) == "/a/b/c/#" );
                CHECK( subscribe_packet.subscriptions( )[1].maximum_qos( ) == protocol::packet::QoS::AT_LEAST_ONCE );

                CHECK( subscribe_packet.subscriptions( )[2].topic_filter( ) == "/a/b/cdd/#" );
                CHECK( subscribe_packet.subscriptions( )[2].maximum_qos( ) == protocol::packet::QoS::EXACTLY_ONCE );
            }
        }
    }
}
