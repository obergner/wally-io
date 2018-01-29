#include "catch.hpp"

#include <cstdint>

#include "io_wally/protocol/common.hpp"

using namespace io_wally::protocol;

TEST_CASE( "MQTT header flags", "[header]" )
{
    SECTION( "all flags are set" )
    {
        const std::uint8_t flgs = 0x0F;
        packet::header_flags under_test( flgs );

        REQUIRE( under_test.dup( ) );
        REQUIRE( under_test.retain( ) );
        REQUIRE( under_test.qos( ) == packet::QoS::RESERVED );
    }

    SECTION( "only flags 0 and 1 are set" )
    {
        const std::uint8_t flgs = 0x03;
        packet::header_flags under_test( flgs );

        REQUIRE( !under_test.dup( ) );
        REQUIRE( under_test.retain( ) );
        REQUIRE( under_test.qos( ) == packet::QoS::AT_LEAST_ONCE );
    }

    SECTION( "only flags 1 and 2 are set" )
    {
        const std::uint8_t flgs = 0x06;
        packet::header_flags under_test( flgs );

        REQUIRE( !under_test.dup( ) );
        REQUIRE( !under_test.retain( ) );
        REQUIRE( under_test.qos( ) == packet::QoS::RESERVED );
    }

    SECTION( "only flags 2 and 3 are set" )
    {
        const std::uint8_t flgs = 0x0C;
        packet::header_flags under_test( flgs );

        REQUIRE( under_test.dup( ) );
        REQUIRE( !under_test.retain( ) );
        REQUIRE( under_test.qos( ) == packet::QoS::EXACTLY_ONCE );
    }
}

TEST_CASE( "An MQTT header is read", "[header]" )
{

    SECTION( "no control packet type bits/no flags are set" )
    {
        const std::uint8_t type_and_flags = 0x00;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::RESERVED1 );
    }

    SECTION( "control packet type bit 4/no flags are set" )
    {
        const std::uint8_t type_and_flags = 0x10;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::CONNECT );
    }

    SECTION( "control packet type bit 5/no flags are set" )
    {
        const std::uint8_t type_and_flags = 0x20;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::CONNACK );
    }

    SECTION( "control packet type bits 4 and 5/no flags are set" )
    {
        const std::uint8_t type_and_flags = 0x30;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::PUBLISH );
    }

    SECTION( "control packet type bit 6/no flags are set" )
    {
        const std::uint8_t type_and_flags = 0x40;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::PUBACK );
    }

    SECTION( "control packet type bits 6 and 4/no flags are set" )
    {
        const std::uint8_t type_and_flags = 0x50;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::PUBREC );
    }

    SECTION( "control packet type bits 6 and 5/no flags are set" )
    {
        const std::uint8_t type_and_flags = 0x60;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::PUBREL );
    }

    SECTION( "control packet type bits 6, 5 and 4/no flags are set" )
    {
        const std::uint8_t type_and_flags = 0x70;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::PUBCOMP );
    }

    SECTION( "control packet type bit 7/no flags are set" )
    {
        const std::uint8_t type_and_flags = 0x80;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::SUBSCRIBE );
    }

    SECTION( "control packet type bits 7 and 4/no flags are set" )
    {
        const std::uint8_t type_and_flags = 0x90;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::SUBACK );
    }

    SECTION( "control packet type bits 7 and 5/no flags are set" )
    {
        const std::uint8_t type_and_flags = 0xA0;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::UNSUBSCRIBE );
    }

    SECTION( "control packet type bits 7, 5 and 4/no flags are set" )
    {
        const std::uint8_t type_and_flags = 0xB0;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::UNSUBACK );
    }

    SECTION( "control packet type bits 7 and 6/no flags are set" )
    {
        const std::uint8_t type_and_flags = 0xC0;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::PINGREQ );
    }

    SECTION( "control packet type bits 7, 6 and 4/no flags are set" )
    {
        const std::uint8_t type_and_flags = 0xD0;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::PINGRESP );
    }

    SECTION( "control packet type bits 7, 6 and 5/no flags are set" )
    {
        const std::uint8_t type_and_flags = 0xE0;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::DISCONNECT );
    }

    SECTION( "all control packet type bits/no flags are set" )
    {
        const std::uint8_t type_and_flags = 0xF0;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::RESERVED2 );
    }

    SECTION( "all control packet type bits/flag bit 0 are set" )
    {
        const std::uint8_t type_and_flags = 0xF1;
        const std::uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.flags( ).retain( ) );
    }
}

SCENARIO( "converting a header byte into Type", "[packets]" )
{
    GIVEN( "a byte encoding type PUBREL with some header flags set" )
    {
        const std::uint8_t input = 0x6F;

        WHEN( "a caller passes that byte into type_of()" )
        {
            const packet::Type type = packet::type_of( input );

            THEN( "it will receive Type::PUBREL" )
            {
                REQUIRE( type == packet::Type::PUBREL );
            }
        }
    }
}

SCENARIO( "writing a Type into a header byte", "[packets]" )
{
    GIVEN( "a header byte with type CONNECT and some flags set" )
    {
        std::uint8_t header_byte = 0x1E;
        const std::uint8_t expected_header_byte = 0x3E;

        WHEN( "a caller passes type PUBLISH and the given header byte into type_into()" )
        {
            packet::type_into( packet::Type::PUBLISH, header_byte );

            THEN( "it will see a correctly updated header byte" )
            {
                REQUIRE( header_byte == expected_header_byte );
            }
        }
    }
}

SCENARIO( "calculating a packet's total length in bytes on the wire", "[packets]" )
{
    GIVEN( "a fixed header with remaining length 126" )
    {
        const std::uint8_t type_and_flags = 0xF1;
        const std::uint32_t remaining_length = 126;
        const packet::header under_test( type_and_flags, remaining_length );

        WHEN( "a caller calls total_length()" )
        {
            const std::uint32_t total_length = under_test.total_length( );

            THEN( "it will see a total length of remaining_length + 2 bytes" )
            {
                REQUIRE( total_length == remaining_length + 2 );
            }
        }
    }

    GIVEN( "a fixed header with remaining length 128" )
    {
        const std::uint8_t type_and_flags = 0xF1;
        const std::uint32_t remaining_length = 128;
        const packet::header under_test( type_and_flags, remaining_length );

        WHEN( "a caller calls total_length()" )
        {
            const std::uint32_t total_length = under_test.total_length( );

            THEN( "it will see a total length of remaining_length + 3 bytes" )
            {
                REQUIRE( total_length == remaining_length + 3 );
            }
        }
    }

    GIVEN( "a fixed header with remaining length 16383" )
    {
        const std::uint8_t type_and_flags = 0xF1;
        const std::uint32_t remaining_length = 16383;
        const packet::header under_test( type_and_flags, remaining_length );

        WHEN( "a caller calls total_length()" )
        {
            const std::uint32_t total_length = under_test.total_length( );

            THEN( "it will see a total length of remaining_length + 3 bytes" )
            {
                REQUIRE( total_length == remaining_length + 3 );
            }
        }
    }

    GIVEN( "a fixed header with remaining length 16384" )
    {
        const std::uint8_t type_and_flags = 0xF1;
        const std::uint32_t remaining_length = 16384;
        const packet::header under_test( type_and_flags, remaining_length );

        WHEN( "a caller calls total_length()" )
        {
            const std::uint32_t total_length = under_test.total_length( );

            THEN( "it will see a total length of remaining_length + 4 bytes" )
            {
                REQUIRE( total_length == remaining_length + 4 );
            }
        }
    }

    GIVEN( "a fixed header with remaining length 2097151" )
    {
        const std::uint8_t type_and_flags = 0xF1;
        const std::uint32_t remaining_length = 2097151;
        const packet::header under_test( type_and_flags, remaining_length );

        WHEN( "a caller calls total_length()" )
        {
            const std::uint32_t total_length = under_test.total_length( );

            THEN( "it will see a total length of remaining_length + 4 bytes" )
            {
                REQUIRE( total_length == remaining_length + 4 );
            }
        }
    }

    GIVEN( "a fixed header with remaining length 2097152" )
    {
        const std::uint8_t type_and_flags = 0xF1;
        const std::uint32_t remaining_length = 2097152;
        const packet::header under_test( type_and_flags, remaining_length );

        WHEN( "a caller calls total_length()" )
        {
            const std::uint32_t total_length = under_test.total_length( );

            THEN( "it will see a total length of remaining_length + 5 bytes" )
            {
                REQUIRE( total_length == remaining_length + 5 );
            }
        }
    }

    GIVEN( "a fixed header with remaining length 268435455" )
    {
        const std::uint8_t type_and_flags = 0xF1;
        const std::uint32_t remaining_length = 268435455;
        const packet::header under_test( type_and_flags, remaining_length );

        WHEN( "a caller calls total_length()" )
        {
            const std::uint32_t total_length = under_test.total_length( );

            THEN( "it will see a total length of remaining_length + 5 bytes" )
            {
                REQUIRE( total_length == remaining_length + 5 );
            }
        }
    }
}
