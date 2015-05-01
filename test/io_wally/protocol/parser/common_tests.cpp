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
            THEN( "that client should see a std::range_error being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.parse( buffer.begin( ), buffer.end( ) ), std::range_error );
            }
        }
    }
}

SCENARIO( "parsing a 16 bit unsigned integer", "[packets]" )
{

    GIVEN( "a buffer of length 0" )
    {
        const std::array<const uint8_t, 0> buffer = {{}};
        uint16_t parsed_int = 0;

        WHEN( "a client passes that buffer into parse_uint16" )
        {
            THEN( "the client should see a std::range_error being thrown" )
            {
                REQUIRE_THROWS_AS( parser::parse_uint16( buffer.begin( ), buffer.cend( ), &parsed_int ),
                                   std::range_error );
            }
        }
    }

    GIVEN( "a buffer of length 1" )
    {
        const std::array<const uint8_t, 1> buffer = {{0x00}};
        uint16_t parsed_int = 0;

        WHEN( "a client passes that buffer into parse_uint16" )
        {
            THEN( "the client should see a std::range_error being thrown" )
            {
                REQUIRE_THROWS_AS( parser::parse_uint16( buffer.begin( ), buffer.cend( ), &parsed_int ),
                                   std::range_error );
            }
        }
    }

    GIVEN( "a buffer of length 2" )
    {
        const uint8_t msb = 0x08;
        const uint8_t lsb = 0xEA;
        const std::array<const uint8_t, 2> buffer = {{msb, lsb}};
        uint16_t parsed_int = 0;

        const uint16_t expected_result = ( msb << 8 ) + lsb;

        WHEN( "a client passes that buffer into parse_uint16" )
        {
            const uint8_t* updated_iterator = parser::parse_uint16( buffer.begin( ), buffer.cend( ), &parsed_int );

            THEN( "the client should receive a correctly decoded result" )
            {
                REQUIRE( parsed_int == expected_result );
            }

            THEN( "the client should receive a correctly updated iterator" )
            {
                REQUIRE( updated_iterator == buffer.begin( ) + 2 );
            }
        }
    }
}

SCENARIO( "parsing a UTF-8 string", "[packets]" )
{

    GIVEN( "a buffer of length 1" )
    {
        const std::array<const char, 1> buffer = {{0x00}};
        char* parsed_string = 0;

        WHEN( "a client passes that buffer into parse_utf8_string" )
        {
            THEN( "the client should see a std::range_error being thrown" )
            {
                REQUIRE_THROWS_AS( parser::parse_utf8_string( buffer.begin( ), buffer.cend( ), &parsed_string ),
                                   std::range_error );
            }
        }
    }

    GIVEN( "a buffer of insufficient length for the contained string" )
    {
        const std::array<const char, 5> buffer = {{0x00, 0x04, 0x61, 0x62, 0x63}};
        char* parsed_string = 0;

        WHEN( "a client passes that buffer into parse_utf8_string" )
        {
            THEN( "the client should see a std::range_error being thrown" )
            {
                REQUIRE_THROWS_AS( parser::parse_utf8_string( buffer.begin( ), buffer.cend( ), &parsed_string ),
                                   std::range_error );
            }
        }
    }

    GIVEN( "a buffer of length 2 containing a correctly encoded empty string" )
    {
        const std::array<const char, 2> buffer = {{0x00, 0x00}};
        char* parsed_string = 0;

        WHEN( "a client passes that buffer into parse_utf8_string" )
        {
            std::array<const char, 2>::iterator new_buffer_start =
                parser::parse_utf8_string( buffer.begin( ), buffer.cend( ), &parsed_string );

            THEN( "the client should receive an empty string" )
            {
                REQUIRE( parsed_string );  // must not be nullptr
                REQUIRE( std::string( parsed_string ) == "" );
            }

            THEN( "the client should receive a correctly updated buffer iterator" )
            {
                REQUIRE( new_buffer_start == buffer.begin( ) + 2 );
            }
        }
    }

    GIVEN( "a buffer of sufficient length containing a correctly encoded non-empty string" )
    {
        const std::array<char, 5> buffer = {{0x00, 0x03, 0x61, 0x62, 0x63}};
        char* parsed_string = 0;

        WHEN( "a client passes that buffer into parse_utf8_string" )
        {
            std::array<const char, 2>::iterator new_buffer_start =
                parser::parse_utf8_string( buffer.begin( ), buffer.cend( ), &parsed_string );

            THEN( "the client should receive an correctly parsed non-empty string" )
            {
                REQUIRE( parsed_string );  // must not be nullptr
                REQUIRE( std::string( parsed_string ) == "abc" );
            }

            THEN( "the client should receive a correctly updated buffer iterator" )
            {
                REQUIRE( new_buffer_start == buffer.begin( ) + 5 );
            }
        }
    }
}
