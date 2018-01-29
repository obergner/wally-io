#include "catch.hpp"

#include <cstdint>
#include <memory>
#include <vector>

#include "io_wally/codec/unsubscribe_packet_decoder.hpp"
#include "io_wally/error/protocol.hpp"

using namespace io_wally;

SCENARIO( "unsubscribe_packet_decoder_impl", "[decoder]" )
{
    const auto under_test = decoder::unsubscribe_packet_decoder_impl{};

    GIVEN( "a well-formed and complete SUSBCRIBE packet body containing 3 subscriptions" )
    {
        const auto type_and_flags = std::uint8_t{( 10 << 4 ) | 2};  // UNSUBSCRIBE
        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/unsubscribe_test.go#L132
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

        WHEN( "a client passes that array into unsubscribe_packet_decoder::decode" )
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

                CHECK( unsubscribe_packet.header( ).type( ) == protocol::packet::Type::UNSUBSCRIBE );

                CHECK( unsubscribe_packet.packet_identifier( ) == 7 );

                CHECK( unsubscribe_packet.topic_filters( ).size( ) == 3 );

                CHECK( unsubscribe_packet.topic_filters( )[0] == "surgemq" );

                CHECK( unsubscribe_packet.topic_filters( )[1] == "/a/b/c/#" );

                CHECK( unsubscribe_packet.topic_filters( )[2] == "/a/b/cdd/#" );
            }
        }
    }

    GIVEN( "a SUSBCRIBE packet body containing no subscriptions" )
    {
        const auto type_and_flags = std::uint8_t{( 10 << 4 ) | 2};  // UNSUBSCRIBE
        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/unsubscribe_test.go#L132
        const auto buffer = std::vector<std::uint8_t>{
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
        };
        const auto frame = decoder::frame{type_and_flags, buffer.begin( ), buffer.end( )};

        WHEN( "a client passes that array into unsubscribe_packet_decoder::decode" )
        {
            THEN( "that client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( frame ), error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a SUSBCRIBE packet with a header having illegal flags set" )
    {
        const auto type_and_flags = std::uint8_t{( 10 << 4 ) | 1};  // UNSUBSCRIBE
        // Shameless act of robbery: https://github.com/surgemq/surgemq/blob/master/message/unsubscribe_test.go#L132
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

        WHEN( "a client passes that header into unsubscribe_packet_decoder::decode" )
        {
            THEN( "that client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( frame ), error::malformed_mqtt_packet );
            }
        }
    }
}
