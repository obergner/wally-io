#include "catch.hpp"

#include <cstdint>
#include <vector>

#include "io_wally/codec/mqtt_packet_decoder.hpp"
#include "io_wally/protocol/protocol.hpp"

using namespace io_wally;

SCENARIO( "mqtt_packet_decoder", "[decoder]" )
{
    const auto under_test = decoder::mqtt_packet_decoder{};

    GIVEN( "a well-formed and complete header byte array of type CONNECT with 1 length byte" )
    {
        const auto type_and_flags = std::uint8_t{1 << 4};  // CONNECT
        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/connect_test.go#L132
        const auto buffer = std::vector<std::uint8_t>{
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
        };
        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that array into mqtt_packet_decoder::parse" )
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

                CHECK( connect_packet.type( ) == protocol::packet::Type::CONNECT );

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
        const auto type_and_flags = std::uint8_t{12 << 4};  // PINGREQ
        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/connect_test.go#L132
        const auto buffer = std::vector<std::uint8_t>{};
        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that array into mqtt_packet_decoder::parse" )
        {
            std::shared_ptr<const protocol::mqtt_packet> result = under_test.decode( frame );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'pingreq' instance" )
            {
                const protocol::mqtt_packet& raw_result = *result;
                const protocol::pingreq& pingreq_packet = static_cast<const protocol::pingreq&>( raw_result );

                CHECK( pingreq_packet.type( ) == protocol::packet::Type::PINGREQ );
            }
        }
    }

    GIVEN( "a well-formed and complete SUSBCRIBE packet body containing 3 subscriptions" )
    {
        const auto type_and_flags = std::uint8_t{( 8 << 4 ) | 2};  // SUBSCRIBE
        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/subscribe_test.go#L132
        const auto buffer = std::vector<std::uint8_t>{
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
            0,  // topic name MSB (0)
            7,  // topic name LSB (7)
            's', 'u', 'r', 'g', 'e', 'm', 'q',
            0,  // QoS
            0,  // topic name MSB (0)
            8,  // topic name LSB (8)
            '/', 'a', '/', 'b', '/', 'c', '/', '#',
            1,   // QoS
            0,   // topic name MSB (0)
            10,  // topic name LSB (10)
            '/', 'a', '/', 'b', '/', 'c', 'd', 'd', '/', '#',
            2  // QoS
        };
        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that array into subscribe_packet_decoder::decode" )
        {
            std::shared_ptr<const protocol::mqtt_packet> result = under_test.decode( frame );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'subscribe' instance with all fields correctly set" )
            {
                const protocol::mqtt_packet& raw_result = *result;
                const protocol::subscribe& subscribe_packet = static_cast<const protocol::subscribe&>( raw_result );

                CHECK( subscribe_packet.type( ) == protocol::packet::Type::SUBSCRIBE );

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

    GIVEN( "a well-formed and complete UNSUSBCRIBE packet body containing 3 topic filters" )
    {
        const auto type_and_flags = std::uint8_t{( 10 << 4 ) | 2};  // UNSUBSCRIBE
        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/subscribe_test.go#L132
        const auto buffer = std::vector<std::uint8_t>{
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
            0,  // topic name MSB (0)
            7,  // topic name LSB (7)
            's', 'u', 'r', 'g', 'e', 'm', 'q',
            0,  // topic name MSB (0)
            8,  // topic name LSB (8)
            '/', 'a', '/', 'b', '/', 'c', '/', '#',
            0,   // topic name MSB (0)
            10,  // topic name LSB (10)
            '/', 'a', '/', 'b', '/', 'c', 'd', 'd', '/', '#',
        };
        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that frame into mqtt_packet_decoder::decode" )
        {
            std::shared_ptr<const protocol::mqtt_packet> result = under_test.decode( frame );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN(
                "it should be able to cast that result to a 'unsubscribe' instance with all fields correctly set" )
            {
                const protocol::mqtt_packet& raw_result = *result;
                const protocol::unsubscribe& unsubscribe_packet =
                    static_cast<const protocol::unsubscribe&>( raw_result );

                CHECK( unsubscribe_packet.type( ) == protocol::packet::Type::UNSUBSCRIBE );

                CHECK( unsubscribe_packet.packet_identifier( ) == 7 );

                CHECK( unsubscribe_packet.topic_filters( ).size( ) == 3 );

                CHECK( unsubscribe_packet.topic_filters( )[0] == "surgemq" );

                CHECK( unsubscribe_packet.topic_filters( )[1] == "/a/b/c/#" );

                CHECK( unsubscribe_packet.topic_filters( )[2] == "/a/b/cdd/#" );
            }
        }
    }

    GIVEN( "a PUBLISH packet body with packet identifier and a header with QoS 1" )
    {
        const auto type_and_flags = std::uint8_t{( 3 << 4 ) | 2};  // PUBLISH + DUP 0, QoS 1, RETAIN 0
        const auto message = std::vector<uint8_t>{'s', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e'};
        // Shameless act of robbery: https://github.com/surgemq/message/blob/master/publish_test.go
        const auto buffer = std::vector<std::uint8_t>{
            0,  // topic name MSB (0)
            7,  // topic name LSB (7)
            's', 'u', 'r', 'g', 'e', 'm', 'q',
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
            's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e',
        };
        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that array into publish_packet_decoder::decode" )
        {
            auto result = under_test.decode( frame );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'publish' instance with all fields correctly set" )
            {
                const auto& raw_result = *result;
                const auto& publish_packet = static_cast<const protocol::publish&>( raw_result );

                CHECK( publish_packet.type( ) == protocol::packet::Type::PUBLISH );

                CHECK( publish_packet.dup( ) == false );
                CHECK( publish_packet.qos( ) == protocol::packet::QoS::AT_LEAST_ONCE );
                CHECK( publish_packet.retain( ) == false );

                CHECK( publish_packet.packet_identifier( ) == 7 );

                CHECK( publish_packet.topic( ) == "surgemq" );

                CHECK( publish_packet.application_message( ) == message );
            }
        }
    }

    GIVEN( "a well-formed PUBACK" )
    {
        const auto type_and_flags = std::uint8_t{( 4 << 4 ) | 0};  // PUBACK
        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/puback_test.go#L132
        const auto buffer = std::vector<std::uint8_t>{
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

    GIVEN( "a well-formed PUBREC" )
    {
        const auto type_and_flags = std::uint8_t{( 5 << 4 ) | 0};  // PUBREC
        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/pubrec_test.go#L132
        const auto buffer = std::vector<std::uint8_t>{
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
        };
        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that array into pubrec_packet_decoder::decode" )
        {
            std::shared_ptr<const protocol::mqtt_packet> result = under_test.decode( frame );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'pubrec' instance with all fields correctly set" )
            {
                const protocol::mqtt_packet& raw_result = *result;
                const protocol::pubrec& pubrec_packet = static_cast<const protocol::pubrec&>( raw_result );

                CHECK( pubrec_packet.type( ) == protocol::packet::Type::PUBREC );

                CHECK( pubrec_packet.packet_identifier( ) == 7 );
            }
        }
    }

    GIVEN( "a well-formed PUBCOMP" )
    {
        const auto type_and_flags = std::uint8_t{( 7 << 4 ) | 0};  // PUBCOMP
        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/pubcomp_test.go#L132
        const auto buffer = std::vector<std::uint8_t>{
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
        };
        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that array into pubcomp_packet_decoder::decode" )
        {
            std::shared_ptr<const protocol::mqtt_packet> result = under_test.decode( frame );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'pubcomp' instance with all fields correctly set" )
            {
                const protocol::mqtt_packet& raw_result = *result;
                const protocol::pubcomp& pubcomp_packet = static_cast<const protocol::pubcomp&>( raw_result );

                CHECK( pubcomp_packet.type( ) == protocol::packet::Type::PUBCOMP );

                CHECK( pubcomp_packet.packet_identifier( ) == 7 );
            }
        }
    }

    GIVEN( "a well-formed PUBREL" )
    {
        const auto type_and_flags = std::uint8_t{( 6 << 4 ) | 2};  // PUBREL
        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/pubrel_test.go#L132
        const auto buffer = std::vector<std::uint8_t>{
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
        };
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

                CHECK( pubrel_packet.type( ) == protocol::packet::Type::PUBREL );

                CHECK( pubrel_packet.packet_identifier( ) == 7 );
            }
        }
    }
}
