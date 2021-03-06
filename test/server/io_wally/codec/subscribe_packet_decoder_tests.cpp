#include "catch.hpp"

#include <cstdint>
#include <memory>
#include <vector>

#include "io_wally/codec/subscribe_packet_decoder.hpp"
#include "io_wally/error/protocol.hpp"

using namespace io_wally;

SCENARIO( "subscribe_packet_decoder_impl", "[decoder]" )
{
    const auto under_test = decoder::subscribe_packet_decoder_impl{};

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

    GIVEN( "a SUSBCRIBE packet body containing no subscriptions" )
    {
        const auto type_and_flags = std::uint8_t{( 8 << 4 ) | 2};  // SUBSCRIBE
        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/subscribe_test.go#L132
        const auto buffer = std::vector<std::uint8_t>{
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
        };
        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that array into subscribe_packet_decoder::decode" )
        {
            THEN( "that client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( frame ), error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a SUSBCRIBE packet with a header having illegal flags set" )
    {
        const auto type_and_flags = std::uint8_t{( 8 << 4 ) | 1};  // SUBSCRIBE
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

        WHEN( "a client passes that header into subscribe_packet_decoder::decode" )
        {
            THEN( "that client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( frame ), error::malformed_mqtt_packet );
            }
        }
    }
}
