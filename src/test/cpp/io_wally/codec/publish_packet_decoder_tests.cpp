#include "catch.hpp"

#include <cstdint>
#include <array>
#include <memory>

#include "io_wally/error/protocol.hpp"
#include "io_wally/codec/publish_packet_decoder.hpp"

using namespace io_wally;

SCENARIO( "publish_packet_decoder", "[decoder]" )
{
    auto under_test = decoder::publish_packet_decoder<const std::uint8_t*>{};

    GIVEN( "a PUBLISH packet body without packet identifier and a header with QoS 0" )
    {
        auto const type_and_flags = std::uint8_t{( 3 << 4 ) | 1};  // PUBLISH + DUP 0, QoS 0, RETAIN 1
        auto const remaining_length = std::uint32_t{21};
        auto const message = std::vector<uint8_t>{'s', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e'};

        auto const fixed_header = protocol::packet::header{type_and_flags, remaining_length};

        // Shameless act of robbery: https://github.com/surgemq/message/blob/master/publish_test.go
        auto const buffer = std::array<std::uint8_t, remaining_length>{{
            0,  // topic name MSB (0)
            7,  // topic name LSB (7)
            's', 'u', 'r', 'g', 'e', 'm', 'q', 's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e',
        }};  /// avoids warning

        WHEN( "a client passes that array into publish_packet_decoder::decode" )
        {
            auto result = under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'publish' instance with all fields correctly set" )
            {
                auto const& raw_result = *result;
                auto const& publish_packet = static_cast<const protocol::publish&>( raw_result );

                CHECK( publish_packet.header( ).type( ) == protocol::packet::Type::PUBLISH );

                CHECK( publish_packet.header( ).flags( ).dup( ) == false );
                CHECK( publish_packet.header( ).flags( ).qos( ) == protocol::packet::QoS::AT_MOST_ONCE );
                CHECK( publish_packet.header( ).flags( ).retain( ) == true );

                CHECK( publish_packet.topic( ) == "surgemq" );

                CHECK( publish_packet.application_message( ) == message );
            }
        }
    }

    GIVEN( "a SUSBCRIBE packet body with packet identifier and a header with QoS 1" )
    {
        auto const type_and_flags = std::uint8_t{( 3 << 4 ) | 2};  // PUBLISH + DUP 0, QoS 1, RETAIN 0
        auto const remaining_length = std::uint32_t{23};
        auto const message = std::vector<uint8_t>{'s', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e'};

        auto const fixed_header = protocol::packet::header{type_and_flags, remaining_length};

        // Shameless act of robbery: https://github.com/surgemq/message/blob/master/publish_test.go
        auto const buffer = std::array<std::uint8_t, remaining_length>{{
            0,  // topic name MSB (0)
            7,  // topic name LSB (7)
            's', 'u', 'r', 'g', 'e', 'm', 'q',
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
            's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e',
        }};  /// avoids warning

        WHEN( "a client passes that array into publish_packet_decoder::decode" )
        {
            auto result = under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'publish' instance with all fields correctly set" )
            {
                auto const& raw_result = *result;
                auto const& publish_packet = static_cast<const protocol::publish&>( raw_result );

                CHECK( publish_packet.header( ).type( ) == protocol::packet::Type::PUBLISH );

                CHECK( publish_packet.header( ).flags( ).dup( ) == false );
                CHECK( publish_packet.header( ).flags( ).qos( ) == protocol::packet::QoS::AT_LEAST_ONCE );
                CHECK( publish_packet.header( ).flags( ).retain( ) == false );

                CHECK( publish_packet.packet_identifier( ) == 7 );

                CHECK( publish_packet.topic( ) == "surgemq" );

                CHECK( publish_packet.application_message( ) == message );
            }
        }
    }

    GIVEN( "a SUSBCRIBE packet body with packet identifier and a header with QoS 2" )
    {
        auto const type_and_flags = std::uint8_t{( 3 << 4 ) | 12};  // PUBLISH + DUP 1, QoS 2, RETAIN 0
        auto const remaining_length = std::uint32_t{23};
        auto const message = std::vector<uint8_t>{'s', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e'};

        auto const fixed_header = protocol::packet::header{type_and_flags, remaining_length};

        // Shameless act of robbery: https://github.com/surgemq/message/blob/master/publish_test.go
        auto const buffer = std::array<std::uint8_t, remaining_length>{{
            0,  // topic name MSB (0)
            7,  // topic name LSB (7)
            's', 'u', 'r', 'g', 'e', 'm', 'q',
            0x23,  // packet ID MSB (0)
            0x34,  // packet ID LSB (7)
            's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e',
        }};  /// avoids warning

        WHEN( "a client passes that array into publish_packet_decoder::decode" )
        {
            auto result = under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) );

            THEN( "that client should receive a non-null mqtt_packet pointer" )
            {
                REQUIRE( result );
            }

            AND_THEN( "it should be able to cast that result to a 'publish' instance with all fields correctly set" )
            {
                auto const& raw_result = *result;
                auto const& publish_packet = static_cast<const protocol::publish&>( raw_result );

                CHECK( publish_packet.header( ).type( ) == protocol::packet::Type::PUBLISH );

                CHECK( publish_packet.header( ).flags( ).dup( ) == true );
                CHECK( publish_packet.header( ).flags( ).qos( ) == protocol::packet::QoS::EXACTLY_ONCE );
                CHECK( publish_packet.header( ).flags( ).retain( ) == false );

                CHECK( publish_packet.packet_identifier( ) == 0x2334 );

                CHECK( publish_packet.topic( ) == "surgemq" );

                CHECK( publish_packet.application_message( ) == message );
            }
        }
    }

    GIVEN( "a SUSBCRIBE packet body with topic '' (ILLEGAL)" )
    {
        auto const type_and_flags = std::uint8_t{( 3 << 4 ) | 2};  // PUBLISH + DUP 0, QoS 1, RETAIN 0
        auto const remaining_length = std::uint32_t{16};

        auto const fixed_header = protocol::packet::header{type_and_flags, remaining_length};

        // Shameless act of robbery: https://github.com/surgemq/message/blob/master/publish_test.go
        const std::array<std::uint8_t, remaining_length> buffer = {{
            0,  // topic name MSB (0)
            0,  // topic name LSB (7)
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
            's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e',
        }};  /// avoids warning

        WHEN( "a client passes that array into publish_packet_decoder::decode" )
        {
            THEN( "that client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a SUSBCRIBE packet body with topic '/wally/#' (ILLEGAL)" )
    {
        auto const type_and_flags = std::uint8_t{( 3 << 4 ) | 2};  // PUBLISH + DUP 0, QoS 1, RETAIN 0
        auto const remaining_length = std::uint32_t{24};

        auto const fixed_header = protocol::packet::header{type_and_flags, remaining_length};

        // Shameless act of robbery: https://github.com/surgemq/message/blob/master/publish_test.go
        auto const buffer = std::array<std::uint8_t, remaining_length>{{
            0,  // topic name MSB (0)
            8,  // topic name LSB (8)
            '/', 'w', 'a', 'l', 'l', 'y', '/', '#',
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
            's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e',
        }};  /// avoids warning

        WHEN( "a client passes that array into publish_packet_decoder::decode" )
        {
            THEN( "that client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a SUSBCRIBE packet body with topic '/wally/+' (ILLEGAL)" )
    {
        auto const type_and_flags = std::uint8_t{( 3 << 4 ) | 2};  // PUBLISH + DUP 0, QoS 1, RETAIN 0
        auto const remaining_length = std::uint32_t{24};

        auto const fixed_header = protocol::packet::header{type_and_flags, remaining_length};

        // Shameless act of robbery: https://github.com/surgemq/message/blob/master/publish_test.go
        auto const buffer = std::array<std::uint8_t, remaining_length>{{
            0,  // topic name MSB (0)
            8,  // topic name LSB (8)
            '/', 'w', 'a', 'l', 'l', 'y', '/', '+',
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
            's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e',
        }};  /// avoids warning

        WHEN( "a client passes that array into publish_packet_decoder::decode" )
        {
            THEN( "that client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a SUSBCRIBE packet body with topic '/wal\0y/g' (ILLEGAL)" )
    {
        auto const type_and_flags = std::uint8_t{( 3 << 4 ) | 2};  // PUBLISH + DUP 0, QoS 1, RETAIN 0
        auto const remaining_length = std::uint32_t{24};

        auto const fixed_header = protocol::packet::header{type_and_flags, remaining_length};

        // Shameless act of robbery: https://github.com/surgemq/message/blob/master/publish_test.go
        auto const buffer = std::array<std::uint8_t, remaining_length>{{
            0,  // topic name MSB (0)
            8,  // topic name LSB (8)
            '/', 'w', 'a', 'l', '\0', 'y', '/', 'g',
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
            's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e',
        }};  /// avoids warning

        WHEN( "a client passes that array into publish_packet_decoder::decode" )
        {
            THEN( "that client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a SUSBCRIBE header with dup flag set and QoS 0  (ILLEGAL)" )
    {
        auto const type_and_flags = std::uint8_t{( 3 << 4 ) | 9};  // PUBLISH + DUP 0, QoS 0, RETAIN 1
        auto const remaining_length = std::uint32_t{24};

        auto const fixed_header = protocol::packet::header{type_and_flags, remaining_length};

        // Shameless act of robbery: https://github.com/surgemq/message/blob/master/publish_test.go
        auto const buffer = std::array<std::uint8_t, remaining_length>{{
            0,  // topic name MSB (0)
            8,  // topic name LSB (8)
            '/', 'w', 'a', 'l', 'l', 'y', '/', 'g',
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
            's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e',
        }};  /// avoids warning

        WHEN( "a client passes that array into publish_packet_decoder::decode" )
        {
            THEN( "that client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a SUSBCRIBE header with QoS 3  (ILLEGAL)" )
    {
        auto const type_and_flags = std::uint8_t{( 3 << 4 ) | 6};  // PUBLISH + DUP 0, QoS 3, RETAIN 0
        auto const remaining_length = std::uint32_t{24};

        auto const fixed_header = protocol::packet::header{type_and_flags, remaining_length};

        // Shameless act of robbery: https://github.com/surgemq/message/blob/master/publish_test.go
        auto const buffer = std::array<std::uint8_t, remaining_length>{{
            0,  // topic name MSB (0)
            8,  // topic name LSB (8)
            '/', 'w', 'a', 'l', 'l', 'y', '/', 'g',
            0,  // packet ID MSB (0)
            7,  // packet ID LSB (7)
            's', 'e', 'n', 'd', ' ', 'm', 'e', ' ', 'h', 'o', 'm', 'e',
        }};  /// avoids warning

        WHEN( "a client passes that array into publish_packet_decoder::decode" )
        {
            THEN( "that client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( fixed_header, buffer.begin( ), buffer.end( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }
}
