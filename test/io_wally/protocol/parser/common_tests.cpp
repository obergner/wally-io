#include "catch.hpp"

#include "io_wally/protocol/parser/common.hpp"

using namespace io_wally::protocol;

SCENARIO( "header_parser", "[parser]" )
{
    parser::header_parser under_test;

    GIVEN( "a well-formed and complete header byte array with 1 length byte" )
    {
        const uint8_t type_and_flags = 0x00;

        const uint8_t first_byte = 0x13;

        const std::array<uint8_t, 2> buffer = {{type_and_flags, first_byte}};  /// avoids warning

        const uint32_t expected_remaining_length = ( first_byte & ~0x80 );

        WHEN( "a client passes that array into header_parser" )
        {
            const parser::header_parser::result<std::array<uint8_t, 2>::const_iterator> result =
                under_test.parse( buffer.begin( ), buffer.end( ) );

            THEN( "that client should receive a valid packet header and a correct buffer iterator" )
            {
                REQUIRE( result.parse_state( ) == parser::COMPLETE );
                REQUIRE( result.is_parsing_complete( ) );
                REQUIRE( result.parsed_header( ).type( ) == packet::RESERVED1 );
                REQUIRE( result.consumed_until( ) == buffer.end( ) );
            }
        }
    }

    GIVEN( "a well-formed and complete header byte array with 4 length bytes" )
    {
        const uint8_t type_and_flags = 0xD0;

        const uint8_t first_byte = 0x8A;
        const uint8_t second_byte = 0xF2;
        const uint8_t third_byte = 0x8B;
        const uint8_t fourth_byte = 0x6F;

        const std::array<uint8_t, 5> buffer = {
            {type_and_flags, first_byte, second_byte, third_byte, fourth_byte}};  /// avoids warning

        const uint32_t expected_remaining_length = 128 * 128 * 128 * ( fourth_byte & ~0x80 ) +
                                                   128 * 128 * ( third_byte & ~0x80 ) + 128 * ( second_byte & ~0x80 ) +
                                                   ( first_byte & ~0x80 );

        WHEN( "a client passes that array into header_parser" )
        {
            const parser::header_parser::result<std::array<uint8_t, 5>::const_iterator> result =
                under_test.parse( buffer.begin( ), buffer.end( ) );

            THEN( "that client should receive a valid packet header and a correct buffer iterator" )
            {
                REQUIRE( result.parse_state( ) == parser::COMPLETE );
                REQUIRE( result.is_parsing_complete( ) );
                REQUIRE( result.parsed_header( ).type( ) == packet::PINGRESP );
                REQUIRE( result.consumed_until( ) == buffer.end( ) );
            }
        }
    }

    GIVEN( "a well-formed and complete header byte array with 4 length bytes divided into two chuncks" )
    {
        const uint8_t type_and_flags = 0xD0;

        const uint8_t first_byte = 0x8A;
        const uint8_t second_byte = 0xF2;
        const uint8_t third_byte = 0x8B;
        const uint8_t fourth_byte = 0x6F;

        const std::array<uint8_t, 5> buffer = {
            {type_and_flags, first_byte, second_byte, third_byte, fourth_byte}};  /// avoids warning

        const uint32_t expected_remaining_length = 128 * 128 * 128 * ( fourth_byte & ~0x80 ) +
                                                   128 * 128 * ( third_byte & ~0x80 ) + 128 * ( second_byte & ~0x80 ) +
                                                   ( first_byte & ~0x80 );

        WHEN( "a client passes the first chunck into header_parser" )
        {
            const parser::header_parser::result<std::array<uint8_t, 5>::const_iterator> result =
                under_test.parse( buffer.begin( ), buffer.begin( ) + 2 );

            THEN( "that client should receive parse_state INCOMPLETE" )
            {
                REQUIRE( result.parse_state( ) == parser::INCOMPLETE );
                REQUIRE( !result.is_parsing_complete( ) );
            }
        }

        WHEN( "a client passes the second chunck into header_parser" )
        {
            const parser::header_parser::result<std::array<uint8_t, 5>::const_iterator> result =
                under_test.parse( buffer.begin( ) + 2, buffer.end( ) );

            THEN( "that client should receive parse_state COMPLETE and a correct buffer iterator" )
            {
                REQUIRE( result.parse_state( ) == parser::COMPLETE );
                REQUIRE( result.is_parsing_complete( ) );
                REQUIRE( result.consumed_until( ) == buffer.end( ) );
            }
        }
    }

    GIVEN( "a mal-formed header byte array with 4 length bytes" )
    {
        const uint8_t type_and_flags = 0xD0;

        const uint8_t first_byte = 0x8A;
        const uint8_t second_byte = 0xF2;
        const uint8_t third_byte = 0x8B;
        const uint8_t fourth_byte = 0x9F;

        const std::array<uint8_t, 5> buffer = {
            {type_and_flags, first_byte, second_byte, third_byte, fourth_byte}};  /// avoids warning

        const uint32_t expected_remaining_length = 128 * 128 * 128 * ( fourth_byte & ~0x80 ) +
                                                   128 * 128 * ( third_byte & ~0x80 ) + 128 * ( second_byte & ~0x80 ) +
                                                   ( first_byte & ~0x80 );

        WHEN( "a client passes that array into header_parser" )
        {
            THEN( "that client should receive parse_state MALFORMED_INPUT" )
            {
                REQUIRE_THROWS_AS( under_test.parse( buffer.begin( ), buffer.end( ) ), std::range_error );
            }
        }
    }
}
