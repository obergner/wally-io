#include "catch.hpp"

#include "io_wally/protocol/codec/common.hpp"

using namespace io_wally::protocol;

SCENARIO( "remaining_length functor", "[packets]" )
{
    parser::remaining_length under_test;

    GIVEN( "a valid remaining length bytes sequence of length 1" )
    {
        const uint8_t remaining_length_byte = 0x7F;
        const uint32_t expected_result = 127;

        WHEN( "a caller passes in one byte" )
        {
            uint32_t actual_result = -1;
            parser::ParseState st = under_test( actual_result, remaining_length_byte );

            THEN( "it should receive parse_state COMPLETE and the correct remaining length" )
            {
                REQUIRE( st == parser::ParseState::COMPLETE );
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
            parser::ParseState st1 = under_test( actual_result, first_byte );
            parser::ParseState st2 = under_test( actual_result, second_byte );

            THEN( "it should on each call receive correct parse_state and in the end the correct remaining length" )
            {
                REQUIRE( st1 == parser::ParseState::INCOMPLETE );
                REQUIRE( st2 == parser::ParseState::COMPLETE );
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
            parser::ParseState st1 = under_test( actual_result, first_byte );
            parser::ParseState st2 = under_test( actual_result, second_byte );
            parser::ParseState st3 = under_test( actual_result, third_byte );

            THEN( "it should on each call receive correct parse_state and in the end the correct remaining length" )
            {
                REQUIRE( st1 == parser::ParseState::INCOMPLETE );
                REQUIRE( st2 == parser::ParseState::INCOMPLETE );
                REQUIRE( st3 == parser::ParseState::COMPLETE );
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
            parser::ParseState st1 = under_test( actual_result, first_byte );
            parser::ParseState st2 = under_test( actual_result, second_byte );
            parser::ParseState st3 = under_test( actual_result, third_byte );
            parser::ParseState st4 = under_test( actual_result, fourth_byte );

            THEN( "it should on each call receive correct parse_state and in the end the correct remaining length" )
            {
                REQUIRE( st1 == parser::ParseState::INCOMPLETE );
                REQUIRE( st2 == parser::ParseState::INCOMPLETE );
                REQUIRE( st3 == parser::ParseState::INCOMPLETE );
                REQUIRE( st4 == parser::ParseState::COMPLETE );
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
            parser::ParseState st1 = under_test( actual_result, first_byte );
            parser::ParseState st2 = under_test( actual_result, second_byte );
            parser::ParseState st3 = under_test( actual_result, third_byte );

            THEN( "it should on the last call receive parse_state OUT_OF_RANGE" )
            {
                REQUIRE( st1 == parser::ParseState::INCOMPLETE );
                REQUIRE( st2 == parser::ParseState::INCOMPLETE );
                REQUIRE( st3 == parser::ParseState::INCOMPLETE );
                REQUIRE_THROWS_AS( under_test( actual_result, fourth_byte ), parser::error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a remaining_length functor that has already run to completion" )
    {
        const uint8_t first_byte = 0x8A;
        const uint8_t second_byte = 0xF2;
        const uint8_t third_byte = 0x8B;
        const uint8_t fourth_byte = 0x6F;

        const uint32_t expected_result = 128 * 128 * 128 * ( fourth_byte & ~0x80 ) +
                                         128 * 128 * ( third_byte & ~0x80 ) + 128 * ( second_byte & ~0x80 ) +
                                         ( first_byte & ~0x80 );

        uint32_t ignored = -1;
        under_test( ignored, 0x8A );
        under_test( ignored, 0xF2 );
        under_test( ignored, 0x8B );
        under_test( ignored, 0x3A );

        WHEN( "a client calls reset() and then reuses that functor" )
        {
            under_test.reset( );

            uint32_t actual_result = -1;
            parser::ParseState st1 = under_test( actual_result, first_byte );
            parser::ParseState st2 = under_test( actual_result, second_byte );
            parser::ParseState st3 = under_test( actual_result, third_byte );
            parser::ParseState st4 = under_test( actual_result, fourth_byte );

            THEN( "it should still receive a correct result" )
            {

                REQUIRE( st1 == parser::ParseState::INCOMPLETE );
                REQUIRE( st2 == parser::ParseState::INCOMPLETE );
                REQUIRE( st3 == parser::ParseState::INCOMPLETE );
                REQUIRE( st4 == parser::ParseState::COMPLETE );
                REQUIRE( actual_result == expected_result );
            }
        }
    }
}

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
                REQUIRE( result.parse_state( ) == parser::ParseState::COMPLETE );
                REQUIRE( result.is_parsing_complete( ) );
                REQUIRE( result.parsed_header( ).type( ) == packet::Type::RESERVED1 );
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
                REQUIRE( result.parse_state( ) == parser::ParseState::COMPLETE );
                REQUIRE( result.is_parsing_complete( ) );
                REQUIRE( result.parsed_header( ).type( ) == packet::Type::PINGRESP );
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
                REQUIRE( result.parse_state( ) == parser::ParseState::INCOMPLETE );
                REQUIRE( !result.is_parsing_complete( ) );
            }
        }

        WHEN( "a client passes the second chunck into header_parser" )
        {
            const parser::header_parser::result<std::array<uint8_t, 5>::const_iterator> result =
                under_test.parse( buffer.begin( ) + 2, buffer.end( ) );

            THEN( "that client should receive parse_state COMPLETE and a correct buffer iterator" )
            {
                REQUIRE( result.parse_state( ) == parser::ParseState::COMPLETE );
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
            THEN( "that client should see a parser::error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.parse( buffer.begin( ), buffer.end( ) ),
                                   parser::error::malformed_mqtt_packet );
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
            THEN( "the client should see a parser::error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( parser::parse_uint16( buffer.begin( ), buffer.cend( ), &parsed_int ),
                                   parser::error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a buffer of length 1" )
    {
        const std::array<const uint8_t, 1> buffer = {{0x00}};
        uint16_t parsed_int = 0;

        WHEN( "a client passes that buffer into parse_uint16" )
        {
            THEN( "the client should see a parser::error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( parser::parse_uint16( buffer.begin( ), buffer.cend( ), &parsed_int ),
                                   parser::error::malformed_mqtt_packet );
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

            AND_THEN( "the client should receive a correctly updated iterator" )
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
            THEN( "the client should see a parser::error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( parser::parse_utf8_string( buffer.begin( ), buffer.cend( ), &parsed_string ),
                                   parser::error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a buffer of insufficient length for the contained string" )
    {
        const std::array<const char, 5> buffer = {{0x00, 0x04, 0x61, 0x62, 0x63}};
        char* parsed_string = 0;

        WHEN( "a client passes that buffer into parse_utf8_string" )
        {
            THEN( "the client should see a parser::error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( parser::parse_utf8_string( buffer.begin( ), buffer.cend( ), &parsed_string ),
                                   parser::error::malformed_mqtt_packet );
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

            AND_THEN( "the client should receive a correctly updated buffer iterator" )
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
