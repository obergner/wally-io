#include "catch.hpp"

#include "io_wally/protocol/common.hpp"

using namespace io_wally::protocol;

TEST_CASE( "MQTT header flags are read", "[header]" )
{

    SECTION( "all flags are set" )
    {
        const uint8_t flgs = 0x0F;
        packet::header_flags under_test( flgs );

        REQUIRE( under_test.dup( ) );
        REQUIRE( under_test.retain( ) );
        REQUIRE( under_test.qos( ) == packet::QoS::RESERVED );
    }

    SECTION( "only flags 0 and 1 are set" )
    {
        const uint8_t flgs = 0x03;
        packet::header_flags under_test( flgs );

        REQUIRE( !under_test.dup( ) );
        REQUIRE( under_test.retain( ) );
        REQUIRE( under_test.qos( ) == packet::QoS::AT_LEAST_ONCE );
    }

    SECTION( "only flags 1 and 2 are set" )
    {
        const uint8_t flgs = 0x06;
        packet::header_flags under_test( flgs );

        REQUIRE( !under_test.dup( ) );
        REQUIRE( !under_test.retain( ) );
        REQUIRE( under_test.qos( ) == packet::QoS::RESERVED );
    }

    SECTION( "only flags 2 and 3 are set" )
    {
        const uint8_t flgs = 0x0C;
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
        const uint8_t type_and_flags = 0x00;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::RESERVED1 );
    }

    SECTION( "control packet type bit 4/no flags are set" )
    {
        const uint8_t type_and_flags = 0x10;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::CONNECT );
    }

    SECTION( "control packet type bit 5/no flags are set" )
    {
        const uint8_t type_and_flags = 0x20;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::CONNACK );
    }

    SECTION( "control packet type bits 4 and 5/no flags are set" )
    {
        const uint8_t type_and_flags = 0x30;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::PUBLISH );
    }

    SECTION( "control packet type bit 6/no flags are set" )
    {
        const uint8_t type_and_flags = 0x40;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::PUBACK );
    }

    SECTION( "control packet type bits 6 and 4/no flags are set" )
    {
        const uint8_t type_and_flags = 0x50;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::PUBREC );
    }

    SECTION( "control packet type bits 6 and 5/no flags are set" )
    {
        const uint8_t type_and_flags = 0x60;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::PUBREL );
    }

    SECTION( "control packet type bits 6, 5 and 4/no flags are set" )
    {
        const uint8_t type_and_flags = 0x70;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::PUBCOMP );
    }

    SECTION( "control packet type bit 7/no flags are set" )
    {
        const uint8_t type_and_flags = 0x80;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::SUBSCRIBE );
    }

    SECTION( "control packet type bits 7 and 4/no flags are set" )
    {
        const uint8_t type_and_flags = 0x90;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::SUBACK );
    }

    SECTION( "control packet type bits 7 and 5/no flags are set" )
    {
        const uint8_t type_and_flags = 0xA0;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::UNSUBSCRIBE );
    }

    SECTION( "control packet type bits 7, 5 and 4/no flags are set" )
    {
        const uint8_t type_and_flags = 0xB0;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::UNSUBACK );
    }

    SECTION( "control packet type bits 7 and 6/no flags are set" )
    {
        const uint8_t type_and_flags = 0xC0;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::PINGREQ );
    }

    SECTION( "control packet type bits 7, 6 and 4/no flags are set" )
    {
        const uint8_t type_and_flags = 0xD0;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::PINGRESP );
    }

    SECTION( "control packet type bits 7, 6 and 5/no flags are set" )
    {
        const uint8_t type_and_flags = 0xE0;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::DISCONNECT );
    }

    SECTION( "all control packet type bits/no flags are set" )
    {
        const uint8_t type_and_flags = 0xF0;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.type( ) == packet::Type::RESERVED2 );
    }

    SECTION( "all control packet type bits/flag bit 0 are set" )
    {
        const uint8_t type_and_flags = 0xF1;
        const uint32_t remaining_length = 325678;
        const packet::header under_test( type_and_flags, remaining_length );

        REQUIRE( under_test.flags( ).retain( ) );
    }
}
