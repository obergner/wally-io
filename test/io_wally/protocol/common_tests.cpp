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
        REQUIRE( under_test.qos( ) == packet::RESERVED );
    }

    SECTION( "only flags 0 and 1 are set" )
    {
        const uint8_t flgs = 0x03;
        packet::header_flags under_test( flgs );

        REQUIRE( !under_test.dup( ) );
        REQUIRE( under_test.retain( ) );
        REQUIRE( under_test.qos( ) == packet::AT_LEAST_ONCE );
    }

    SECTION( "only flags 1 and 2 are set" )
    {
        const uint8_t flgs = 0x06;
        packet::header_flags under_test( flgs );

        REQUIRE( !under_test.dup( ) );
        REQUIRE( !under_test.retain( ) );
        REQUIRE( under_test.qos( ) == packet::RESERVED );
    }

    SECTION( "only flags 2 and 3 are set" )
    {
        const uint8_t flgs = 0x0C;
        packet::header_flags under_test( flgs );

        REQUIRE( under_test.dup( ) );
        REQUIRE( !under_test.retain( ) );
        REQUIRE( under_test.qos( ) == packet::EXACTLY_ONCE );
    }
}

TEST_CASE( "An MQTT header is read", "[header]" )
{

    SECTION( "no control packet type bits/no flags are set" )
    {
        const uint8_t type_and_flags = 0x00;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.type( ) == packet::RESERVED1 );
    }

    SECTION( "control packet type bit 4/no flags are set" )
    {
        const uint8_t type_and_flags = 0x10;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.type( ) == packet::CONNECT );
    }

    SECTION( "control packet type bit 5/no flags are set" )
    {
        const uint8_t type_and_flags = 0x20;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.type( ) == packet::CONNACK );
    }

    SECTION( "control packet type bits 4 and 5/no flags are set" )
    {
        const uint8_t type_and_flags = 0x30;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.type( ) == packet::PUBLISH );
    }

    SECTION( "control packet type bit 6/no flags are set" )
    {
        const uint8_t type_and_flags = 0x40;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.type( ) == packet::PUBACK );
    }

    SECTION( "control packet type bits 6 and 4/no flags are set" )
    {
        const uint8_t type_and_flags = 0x50;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.type( ) == packet::PUBREC );
    }

    SECTION( "control packet type bits 6 and 5/no flags are set" )
    {
        const uint8_t type_and_flags = 0x60;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.type( ) == packet::PUBREL );
    }

    SECTION( "control packet type bits 6, 5 and 4/no flags are set" )
    {
        const uint8_t type_and_flags = 0x70;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.type( ) == packet::PUBCOMP );
    }

    SECTION( "control packet type bit 7/no flags are set" )
    {
        const uint8_t type_and_flags = 0x80;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.type( ) == packet::SUBSCRIBE );
    }

    SECTION( "control packet type bits 7 and 4/no flags are set" )
    {
        const uint8_t type_and_flags = 0x90;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.type( ) == packet::SUBACK );
    }

    SECTION( "control packet type bits 7 and 5/no flags are set" )
    {
        const uint8_t type_and_flags = 0xA0;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.type( ) == packet::UNSUBSCRIBE );
    }

    SECTION( "control packet type bits 7, 5 and 4/no flags are set" )
    {
        const uint8_t type_and_flags = 0xB0;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.type( ) == packet::UNSUBACK );
    }

    SECTION( "control packet type bits 7 and 6/no flags are set" )
    {
        const uint8_t type_and_flags = 0xC0;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.type( ) == packet::PINGREQ );
    }

    SECTION( "control packet type bits 7, 6 and 4/no flags are set" )
    {
        const uint8_t type_and_flags = 0xD0;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.type( ) == packet::PINGRESP );
    }

    SECTION( "control packet type bits 7, 6 and 5/no flags are set" )
    {
        const uint8_t type_and_flags = 0xE0;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.type( ) == packet::DISCONNECT );
    }

    SECTION( "all control packet type bits/no flags are set" )
    {
        const uint8_t type_and_flags = 0xF0;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.type( ) == packet::RESERVED2 );
    }

    SECTION( "all control packet type bits/flag bit 0 are set" )
    {
        const uint8_t type_and_flags = 0xF1;
        const packet::header under_test( type_and_flags );

        REQUIRE( under_test.flags( ).retain( ) );
    }
}

SCENARIO( "remaining_length functor", "[packets]" )
{
    packet::remaining_length under_test;

    GIVEN( "a valid remaining length bytes sequence of length 1" )
    {
        const uint8_t remaining_length_byte = 0x7F;
        const uint32_t expected_result = 127;

        WHEN( "a caller passes in one byte" )
        {
            uint32_t actual_result = -1;
            packet::remaining_length::parse_state st = under_test( actual_result, remaining_length_byte );

            THEN( "it should receive parse_state COMPLETE and the correct remaining length" )
            {
                REQUIRE( st == packet::remaining_length::COMPLETE );
                REQUIRE( actual_result == expected_result );
            }
        }
    }

    GIVEN( "a valid remaining length bytes sequence of length 2" )
    {
        const uint8_t first_byte = 0x9F;
        const uint8_t second_byte = 0x6f;

        const uint32_t expected_result = 128 * ( second_byte & ~0x80 ) + ( first_byte & ~0x80 );

        WHEN( "a caller passes in all bytes" )
        {
            uint32_t actual_result = -1;
            packet::remaining_length::parse_state st1 = under_test( actual_result, first_byte );
            packet::remaining_length::parse_state st2 = under_test( actual_result, second_byte );

            THEN( "it should on each call receive correct parse_state and in the end the correct remaining length" )
            {
                REQUIRE( st1 == packet::remaining_length::INCOMPLETE );
                REQUIRE( st2 == packet::remaining_length::COMPLETE );
                REQUIRE( actual_result == expected_result );
            }
        }
    }

    GIVEN( "a valid remaining length bytes sequence of length 3" )
    {
        const uint8_t first_byte = 0x8A;
        const uint8_t second_byte = 0xF2;
        const uint8_t third_byte = 0x5A;

        const uint32_t expected_result =
            128 * 128 * ( third_byte & ~0x80 ) + 128 * ( second_byte & ~0x80 ) + ( first_byte & ~0x80 );

        WHEN( "a caller passes in all bytes" )
        {
            uint32_t actual_result = -1;
            packet::remaining_length::parse_state st1 = under_test( actual_result, first_byte );
            packet::remaining_length::parse_state st2 = under_test( actual_result, second_byte );
            packet::remaining_length::parse_state st3 = under_test( actual_result, third_byte );

            THEN( "it should on each call receive correct parse_state and in the end the correct remaining length" )
            {
                REQUIRE( st1 == packet::remaining_length::INCOMPLETE );
                REQUIRE( st2 == packet::remaining_length::INCOMPLETE );
                REQUIRE( st3 == packet::remaining_length::COMPLETE );
                REQUIRE( actual_result == expected_result );
            }
        }
    }

    GIVEN( "a valid remaining length bytes sequence of length 4" )
    {
        const uint8_t first_byte = 0x8A;
        const uint8_t second_byte = 0xF2;
        const uint8_t third_byte = 0x8B;
        const uint8_t fourth_byte = 0x6F;

        const uint32_t expected_result = 128 * 128 * 128 * ( fourth_byte & ~0x80 ) +
                                         128 * 128 * ( third_byte & ~0x80 ) + 128 * ( second_byte & ~0x80 ) +
                                         ( first_byte & ~0x80 );

        WHEN( "a caller passes in all bytes" )
        {
            uint32_t actual_result = -1;
            packet::remaining_length::parse_state st1 = under_test( actual_result, first_byte );
            packet::remaining_length::parse_state st2 = under_test( actual_result, second_byte );
            packet::remaining_length::parse_state st3 = under_test( actual_result, third_byte );
            packet::remaining_length::parse_state st4 = under_test( actual_result, fourth_byte );

            THEN( "it should on each call receive correct parse_state and in the end the correct remaining length" )
            {
                REQUIRE( st1 == packet::remaining_length::INCOMPLETE );
                REQUIRE( st2 == packet::remaining_length::INCOMPLETE );
                REQUIRE( st3 == packet::remaining_length::INCOMPLETE );
                REQUIRE( st4 == packet::remaining_length::COMPLETE );
                REQUIRE( actual_result == expected_result );
            }
        }
    }

    GIVEN( "an invalid remaining length bytes sequence of length 4" )
    {
        const uint8_t first_byte = 0x8A;
        const uint8_t second_byte = 0xF2;
        const uint8_t third_byte = 0x8B;
        const uint8_t fourth_byte = 0x80;

        WHEN( "a caller passes in all bytes" )
        {
            uint32_t actual_result = -1;
            packet::remaining_length::parse_state st1 = under_test( actual_result, first_byte );
            packet::remaining_length::parse_state st2 = under_test( actual_result, second_byte );
            packet::remaining_length::parse_state st3 = under_test( actual_result, third_byte );
            packet::remaining_length::parse_state st4 = under_test( actual_result, fourth_byte );

            THEN( "it should on the last call receive parse_state OUT_OF_RANGE" )
            {
                REQUIRE( st1 == packet::remaining_length::INCOMPLETE );
                REQUIRE( st2 == packet::remaining_length::INCOMPLETE );
                REQUIRE( st3 == packet::remaining_length::INCOMPLETE );
                REQUIRE( st4 == packet::remaining_length::OUT_OF_RANGE );
            }
        }
    }
}
