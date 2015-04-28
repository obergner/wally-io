#include "catch.hpp"

#include "io_wally/protocol/parser/common.hpp"

using namespace io_wally::protocol;

SCENARIO( "header_parser", "[parser]" )
{
    parser::header_parser under_test;

    GIVEN( "a well-formed and complete header byte array" )
    {
        const uint8_t type_and_flags = 0xD0;

        const uint8_t first_byte = 0x8A;
        const uint8_t second_byte = 0xF2;
        const uint8_t third_byte = 0x8B;
        const uint8_t fourth_byte = 0x6F;

        const std::array<uint8_t, 5> buffer = {
            {type_and_flags, first_byte, second_byte, third_byte, fourth_byte}};  /// avoids warning

        const packet::type expected_type = packet::PINGRESP;

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
                REQUIRE( !result.is_input_malformed( ) );
                REQUIRE( result.parsed_header( ).type( ) == expected_type );
                REQUIRE( result.consumed_until( ) == buffer.end( ) );
            }
        }
    }
}
