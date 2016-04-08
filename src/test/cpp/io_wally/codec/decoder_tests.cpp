#include "catch.hpp"

#include <cstdint>
#include <array>
#include <string>
#include <tuple>
#include <algorithm>

#include <boost/system/error_code.hpp>

#include "io_wally/protocol/common.hpp"
#include "io_wally/error/protocol.hpp"
#include "io_wally/codec/decoder.hpp"

using namespace io_wally;

SCENARIO( "remaining_length functor", "[packets]" )
{
    decoder::remaining_length under_test;

    GIVEN( "a valid remaining length bytes sequence of length 1" )
    {
        const std::uint8_t remaining_length_byte = 0x7F;
        const std::uint32_t expected_result = 127;

        WHEN( "a caller passes in one byte" )
        {
            std::uint32_t actual_result = -1;
            decoder::ParseState st = under_test( actual_result, remaining_length_byte );

            THEN( "it should receive parse_state COMPLETE and the correct remaining length" )
            {
                REQUIRE( st == decoder::ParseState::COMPLETE );
                REQUIRE( actual_result == expected_result );
            }
        }
    }

    GIVEN( "a valid remaining length bytes sequence of length 2" )
    {
        const std::uint8_t first_byte = 0x9F;
        const std::uint8_t second_byte = 0x6f;

        const std::uint32_t expected_result = 128 * ( second_byte & ~0x80 ) + ( first_byte & ~0x80 );

        WHEN( "a caller passes in all bytes" )
        {
            std::uint32_t actual_result = -1;
            decoder::ParseState st1 = under_test( actual_result, first_byte );
            decoder::ParseState st2 = under_test( actual_result, second_byte );

            THEN( "it should on each call receive correct parse_state and in the end the correct remaining length" )
            {
                REQUIRE( st1 == decoder::ParseState::INCOMPLETE );
                REQUIRE( st2 == decoder::ParseState::COMPLETE );
                REQUIRE( actual_result == expected_result );
            }
        }
    }

    GIVEN( "a valid remaining length bytes sequence of length 3" )
    {
        const std::uint8_t first_byte = 0x8A;
        const std::uint8_t second_byte = 0xF2;
        const std::uint8_t third_byte = 0x5A;

        const std::uint32_t expected_result =
            128 * 128 * ( third_byte & ~0x80 ) + 128 * ( second_byte & ~0x80 ) + ( first_byte & ~0x80 );

        WHEN( "a caller passes in all bytes" )
        {
            std::uint32_t actual_result = -1;
            decoder::ParseState st1 = under_test( actual_result, first_byte );
            decoder::ParseState st2 = under_test( actual_result, second_byte );
            decoder::ParseState st3 = under_test( actual_result, third_byte );

            THEN( "it should on each call receive correct parse_state and in the end the correct remaining length" )
            {
                REQUIRE( st1 == decoder::ParseState::INCOMPLETE );
                REQUIRE( st2 == decoder::ParseState::INCOMPLETE );
                REQUIRE( st3 == decoder::ParseState::COMPLETE );
                REQUIRE( actual_result == expected_result );
            }
        }
    }

    GIVEN( "a valid remaining length bytes sequence of length 4" )
    {
        const std::uint8_t first_byte = 0x8A;
        const std::uint8_t second_byte = 0xF2;
        const std::uint8_t third_byte = 0x8B;
        const std::uint8_t fourth_byte = 0x6F;

        const std::uint32_t expected_result = 128 * 128 * 128 * ( fourth_byte & ~0x80 ) +
                                              128 * 128 * ( third_byte & ~0x80 ) + 128 * ( second_byte & ~0x80 ) +
                                              ( first_byte & ~0x80 );

        WHEN( "a caller passes in all bytes" )
        {
            std::uint32_t actual_result = -1;
            decoder::ParseState st1 = under_test( actual_result, first_byte );
            decoder::ParseState st2 = under_test( actual_result, second_byte );
            decoder::ParseState st3 = under_test( actual_result, third_byte );
            decoder::ParseState st4 = under_test( actual_result, fourth_byte );

            THEN( "it should on each call receive correct parse_state and in the end the correct remaining length" )
            {
                REQUIRE( st1 == decoder::ParseState::INCOMPLETE );
                REQUIRE( st2 == decoder::ParseState::INCOMPLETE );
                REQUIRE( st3 == decoder::ParseState::INCOMPLETE );
                REQUIRE( st4 == decoder::ParseState::COMPLETE );
                REQUIRE( actual_result == expected_result );
            }
        }
    }

    GIVEN( "an invalid remaining length bytes sequence of length 4" )
    {
        const std::uint8_t first_byte = 0x8A;
        const std::uint8_t second_byte = 0xF2;
        const std::uint8_t third_byte = 0x8B;
        const std::uint8_t fourth_byte = 0x80;

        WHEN( "a caller passes in all bytes" )
        {
            std::uint32_t actual_result = -1;
            decoder::ParseState st1 = under_test( actual_result, first_byte );
            decoder::ParseState st2 = under_test( actual_result, second_byte );
            decoder::ParseState st3 = under_test( actual_result, third_byte );

            THEN( "it should on the last call receive parse_state OUT_OF_RANGE" )
            {
                REQUIRE( st1 == decoder::ParseState::INCOMPLETE );
                REQUIRE( st2 == decoder::ParseState::INCOMPLETE );
                REQUIRE( st3 == decoder::ParseState::INCOMPLETE );
                REQUIRE_THROWS_AS( under_test( actual_result, fourth_byte ), error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a remaining_length functor that has already run to completion" )
    {
        const std::uint8_t first_byte = 0x8A;
        const std::uint8_t second_byte = 0xF2;
        const std::uint8_t third_byte = 0x8B;
        const std::uint8_t fourth_byte = 0x6F;

        const std::uint32_t expected_result = 128 * 128 * 128 * ( fourth_byte & ~0x80 ) +
                                              128 * 128 * ( third_byte & ~0x80 ) + 128 * ( second_byte & ~0x80 ) +
                                              ( first_byte & ~0x80 );

        std::uint32_t ignored = -1;
        under_test( ignored, 0x8A );
        under_test( ignored, 0xF2 );
        under_test( ignored, 0x8B );
        under_test( ignored, 0x3A );

        WHEN( "a client calls reset() and then reuses that functor" )
        {
            under_test.reset( );

            std::uint32_t actual_result = -1;
            decoder::ParseState st1 = under_test( actual_result, first_byte );
            decoder::ParseState st2 = under_test( actual_result, second_byte );
            decoder::ParseState st3 = under_test( actual_result, third_byte );
            decoder::ParseState st4 = under_test( actual_result, fourth_byte );

            THEN( "it should still receive a correct result" )
            {

                REQUIRE( st1 == decoder::ParseState::INCOMPLETE );
                REQUIRE( st2 == decoder::ParseState::INCOMPLETE );
                REQUIRE( st3 == decoder::ParseState::INCOMPLETE );
                REQUIRE( st4 == decoder::ParseState::COMPLETE );
                REQUIRE( actual_result == expected_result );
            }
        }
    }
}

SCENARIO( "frame_reader", "[packets]" )
{
    auto buffer = std::vector<uint8_t>( 512 );
    auto under_test = decoder::frame_reader{buffer};
    const auto ec_success = boost::system::errc::make_error_code( boost::system::errc::success );

    GIVEN( "a well-formed packet header of length two encoding a remaining length of 0" )
    {
        const auto serialized_header = std::array<uint8_t, 2>{{0x01 << 4, 0x00}};
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor" )
        {
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            THEN( "it should receive 0" )
            {
                REQUIRE( frame_remainder == 0 );
            }
        }
    }

    GIVEN( "a well-formed packet header of length two encoding a remaining length of 127" )
    {
        const auto serialized_header = std::array<uint8_t, 2>{{0x05 << 4, 0x7F}};
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor for the first time" )
        {
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            THEN( "it should receive 127" )
            {
                REQUIRE( frame_remainder == 127 );
            }
        }
    }

    GIVEN( "a well-formed packet header of length three encoding a remaining length of 128" )
    {
        const auto serialized_header = std::array<uint8_t, 3>{{0x07 << 4, 0x80, 0x01}};
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor for the first time" )
        {
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            THEN( "it should receive 128" )
            {
                REQUIRE( frame_remainder == 128 );
            }
        }
    }

    GIVEN( "a well-formed packet header of length three encoding a remaining length of 127 * 128 + 127" )
    {
        const auto serialized_header = std::array<uint8_t, 3>{{0x07 << 4, 0xFF, 0x7F}};
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor for the first time" )
        {
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            THEN( "it should receive 127 * 128 + 127" )
            {
                REQUIRE( frame_remainder == 127 * 128 + 127 );
            }
        }
    }

    GIVEN( "a well-formed packet header of length four encoding a remaining length of 128 * 128" )
    {
        const auto serialized_header = std::array<uint8_t, 4>{{0x07 << 4, 0x80, 0x80, 0x01}};
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor for the first time" )
        {
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            THEN( "it should receive 128 * 128" )
            {
                REQUIRE( frame_remainder == 128 * 128 );
            }
        }
    }

    GIVEN(
        "a well-formed packet header of length four encoding a remaining length of 127 * 128 * 128 + 127 * 128 + 127" )
    {
        const auto serialized_header = std::array<uint8_t, 4>{{0x07 << 4, 0xFF, 0xFF, 0x7F}};
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor for the first time" )
        {
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            THEN( "it should receive 127 * 128 * 128 + 127 * 128 + 127" )
            {
                REQUIRE( frame_remainder == 127 * 128 * 128 + 127 * 128 + 127 );
            }
        }
    }

    GIVEN( "a well-formed packet header of length five encoding a remaining length of 128 * 128 * 128" )
    {
        const auto serialized_header = std::array<uint8_t, 5>{{0x07 << 4, 0x80, 0x80, 0x80, 0x01}};
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor for the first time" )
        {
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            THEN( "it should receive 128 * 128 * 128" )
            {
                REQUIRE( frame_remainder == 128 * 128 * 128 );
            }
        }
    }

    GIVEN( "a well-formed packet header of length five encoding maximum allowed remaining length" )
    {
        const auto serialized_header = std::array<uint8_t, 5>{{0x07 << 4, 0xFF, 0xFF, 0xFF, 0x7F}};
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor for the first time" )
        {
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            THEN( "it should receive maximum allowed remaining length" )
            {
                REQUIRE( frame_remainder == io_wally::protocol::packet::MAX_ALLOWED_PACKET_LENGTH );
            }
        }
    }

    GIVEN( "a mal-formed packet header of length six encoding maximum allowed remaining length + 1" )
    {
        const auto serialized_header = std::array<uint8_t, 6>{{0x07 << 4, 0xFF, 0xFF, 0xFF, 0xFF, 0x01}};
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor for the first time" )
        {
            THEN( "it should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test( ec_success, serialized_header.size( ) ), error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a well-formed packet header with maximum allowed remaining length" )
    {
        const auto serialized_header = std::array<uint8_t, 5>{{0x07 << 4, 0xFF, 0xFF, 0xFF, 0x7F}};
        const auto total_frame_length =
            serialized_header.size( ) + io_wally::protocol::packet::MAX_ALLOWED_PACKET_LENGTH;
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor for the first time" )
        {
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            THEN( "it should receive maximum allowed remaining length" )
            {
                REQUIRE( frame_remainder == io_wally::protocol::packet::MAX_ALLOWED_PACKET_LENGTH );
            }
        }

        WHEN( "a caller repeatedly calls frame_reader as a functor while buffer is being filled" )
        {
            const auto step_size = std::size_t{100000};  // Large step size so that test runs faster
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            auto bytes_transferred = serialized_header.size( );

            THEN( "on each call it should receive correct number of bytes still to be read" )
            {
                auto frame_remainder = io_wally::protocol::packet::MAX_ALLOWED_PACKET_LENGTH;
                while ( frame_remainder > 0 )
                {
                    auto insert_count = std::min( step_size, total_frame_length - bytes_transferred );
                    buffer.insert( std::end( buffer ), insert_count, 0x01 );
                    bytes_transferred += insert_count;

                    frame_remainder = under_test( ec_success, bytes_transferred );
                    REQUIRE( frame_remainder == total_frame_length - bytes_transferred );
                }

                REQUIRE( under_test( ec_success, bytes_transferred ) == 0 );
            }
        }
    }

    GIVEN( "a partial packet header of eventual length 4" )
    {
        const auto serialized_header = std::array<uint8_t, 2>{{0x07 << 4, 0xFF}};
        const auto ec_success = boost::system::errc::make_error_code( boost::system::errc::success );
        const auto actual_remaining_length = 127 * 128 * 128 + 127 * 128 + 127;

        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );
        auto bytes_transferred = serialized_header.size( );

        WHEN( "a caller repeatedly calls frame_reader as a functor" )
        {
            THEN( "it should receive 1 until header is complete" )
            {
                REQUIRE( under_test( ec_success, bytes_transferred ) == 1 );

                buffer.insert( std::begin( buffer ) + bytes_transferred++, 0xFF );
                REQUIRE( under_test( ec_success, bytes_transferred ) == 1 );

                buffer.insert( std::begin( buffer ) + bytes_transferred++, 0x7F );
                REQUIRE( under_test( ec_success, bytes_transferred ) == actual_remaining_length );
            }
        }
    }
}

SCENARIO( "header_decoder", "[decoder]" )
{
    decoder::header_decoder under_test;

    GIVEN( "a well-formed and complete header byte array with 1 length byte" )
    {
        const std::uint8_t type_and_flags = 0x00;

        const std::uint8_t first_byte = 0x13;

        const std::array<std::uint8_t, 2> buffer = {{type_and_flags, first_byte}};  /// avoids warning

        const std::uint32_t expected_remaining_length = ( first_byte & ~0x80 );

        WHEN( "a client passes that array into header_decoder" )
        {
            const decoder::header_decoder::result<std::array<std::uint8_t, 2>::const_iterator> result =
                under_test.decode( buffer.begin( ), buffer.end( ) );

            THEN( "that client should receive a valid packet header and a correct buffer iterator" )
            {
                REQUIRE( result.parse_state( ) == decoder::ParseState::COMPLETE );
                REQUIRE( result.is_parsing_complete( ) );
                REQUIRE( result.parsed_header( ).type( ) == protocol::packet::Type::RESERVED1 );
                REQUIRE( result.consumed_until( ) == buffer.end( ) );
            }
        }
    }

    GIVEN( "a well-formed and complete header byte array with 4 length bytes" )
    {
        const std::uint8_t type_and_flags = 0xD0;

        const std::uint8_t first_byte = 0x8A;
        const std::uint8_t second_byte = 0xF2;
        const std::uint8_t third_byte = 0x8B;
        const std::uint8_t fourth_byte = 0x6F;

        const std::array<std::uint8_t, 5> buffer = {
            {type_and_flags, first_byte, second_byte, third_byte, fourth_byte}};  /// avoids warning

        const std::uint32_t expected_remaining_length = 128 * 128 * 128 * ( fourth_byte & ~0x80 ) +
                                                        128 * 128 * ( third_byte & ~0x80 ) +
                                                        128 * ( second_byte & ~0x80 ) + ( first_byte & ~0x80 );

        WHEN( "a client passes that array into header_decoder" )
        {
            const decoder::header_decoder::result<std::array<std::uint8_t, 5>::const_iterator> result =
                under_test.decode( buffer.begin( ), buffer.end( ) );

            THEN( "that client should receive a valid packet header and a correct buffer iterator" )
            {
                REQUIRE( result.parse_state( ) == decoder::ParseState::COMPLETE );
                REQUIRE( result.is_parsing_complete( ) );
                REQUIRE( result.parsed_header( ).type( ) == protocol::packet::Type::PINGRESP );
                REQUIRE( result.consumed_until( ) == buffer.end( ) );
            }
        }
    }

    GIVEN( "a well-formed and complete header byte array with 4 length bytes divided into two chuncks" )
    {
        const std::uint8_t type_and_flags = 0xD0;

        const std::uint8_t first_byte = 0x8A;
        const std::uint8_t second_byte = 0xF2;
        const std::uint8_t third_byte = 0x8B;
        const std::uint8_t fourth_byte = 0x6F;

        const std::array<std::uint8_t, 5> buffer = {
            {type_and_flags, first_byte, second_byte, third_byte, fourth_byte}};  /// avoids warning

        const std::uint32_t expected_remaining_length = 128 * 128 * 128 * ( fourth_byte & ~0x80 ) +
                                                        128 * 128 * ( third_byte & ~0x80 ) +
                                                        128 * ( second_byte & ~0x80 ) + ( first_byte & ~0x80 );

        WHEN( "a client passes the first chunck into header_decoder" )
        {
            const decoder::header_decoder::result<std::array<std::uint8_t, 5>::const_iterator> result =
                under_test.decode( buffer.begin( ), buffer.begin( ) + 2 );

            THEN( "that client should receive parse_state INCOMPLETE" )
            {
                REQUIRE( result.parse_state( ) == decoder::ParseState::INCOMPLETE );
                REQUIRE( !result.is_parsing_complete( ) );
            }
        }

        WHEN( "a client passes the second chunck into header_decoder" )
        {
            const decoder::header_decoder::result<std::array<std::uint8_t, 5>::const_iterator> result =
                under_test.decode( buffer.begin( ) + 2, buffer.end( ) );

            THEN( "that client should receive parse_state COMPLETE and a correct buffer iterator" )
            {
                REQUIRE( result.parse_state( ) == decoder::ParseState::COMPLETE );
                REQUIRE( result.is_parsing_complete( ) );
                REQUIRE( result.consumed_until( ) == buffer.end( ) );
            }
        }
    }

    GIVEN( "a mal-formed header byte array with 4 length bytes" )
    {
        const std::uint8_t type_and_flags = 0xD0;

        const std::uint8_t first_byte = 0x8A;
        const std::uint8_t second_byte = 0xF2;
        const std::uint8_t third_byte = 0x8B;
        const std::uint8_t fourth_byte = 0x9F;

        const std::array<std::uint8_t, 5> buffer = {
            {type_and_flags, first_byte, second_byte, third_byte, fourth_byte}};  /// avoids warning

        const std::uint32_t expected_remaining_length = 128 * 128 * 128 * ( fourth_byte & ~0x80 ) +
                                                        128 * 128 * ( third_byte & ~0x80 ) +
                                                        128 * ( second_byte & ~0x80 ) + ( first_byte & ~0x80 );

        WHEN( "a client passes that array into header_decoder" )
        {
            THEN( "that client should see a error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( under_test.decode( buffer.begin( ), buffer.end( ) ), error::malformed_mqtt_packet );
            }
        }
    }
}

SCENARIO( "parsing a 16 bit unsigned integer", "[packets]" )
{

    GIVEN( "a buffer of length 0" )
    {
        const std::array<const std::uint8_t, 0> buffer = {{}};

        WHEN( "a client passes that buffer into parse_std::uint16" )
        {
            THEN( "the client should see a error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( decoder::decode_uint16( buffer.begin( ), buffer.cend( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a buffer of length 1" )
    {
        const std::array<const std::uint8_t, 1> buffer = {{0x00}};

        WHEN( "a client passes that buffer into parse_std::uint16" )
        {
            THEN( "the client should see a error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( decoder::decode_uint16( buffer.begin( ), buffer.cend( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a buffer of length 2" )
    {
        const std::uint8_t msb = 0x08;
        const std::uint8_t lsb = 0xEA;
        const std::array<const std::uint8_t, 2> buffer = {{msb, lsb}};
        std::uint16_t parsed_int = 0;

        const std::uint16_t expected_result = ( msb << 8 ) + lsb;

        WHEN( "a client passes that buffer into parse_std::uint16" )
        {
            const std::uint8_t* updated_iterator;
            std::tie( updated_iterator, parsed_int ) = decoder::decode_uint16( buffer.begin( ), buffer.cend( ) );

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

        WHEN( "a client passes that buffer into decode_utf8_string" )
        {
            THEN( "the client should see a error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( decoder::decode_utf8_string( buffer.begin( ), buffer.cend( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a buffer of insufficient length for the contained string" )
    {
        const std::array<const char, 5> buffer = {{0x00, 0x04, 0x61, 0x62, 0x63}};

        WHEN( "a client passes that buffer into decode_utf8_string" )
        {
            THEN( "the client should see a error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( decoder::decode_utf8_string( buffer.begin( ), buffer.cend( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a buffer of length 2 containing a correctly encoded empty string" )
    {
        const std::array<const char, 3> buffer = {{0x00, 0x00, 0x00}};
        std::string parsed_string;

        WHEN( "a client passes that buffer into decode_utf8_string" )
        {
            std::array<const char, 3>::iterator new_buffer_start;
            std::tie( new_buffer_start, parsed_string ) =
                decoder::decode_utf8_string( buffer.begin( ), buffer.cend( ) );

            THEN( "the client should receive an empty string" )
            {
                REQUIRE( parsed_string == "" );
            }

            AND_THEN( "the client should receive a correctly updated buffer iterator" )
            {
                REQUIRE( new_buffer_start == buffer.begin( ) + 2 );
            }
        }
    }

    GIVEN( "a buffer of sufficient length containing a correctly encoded non-empty string" )
    {
        const std::array<char, 6> buffer = {{0x00, 0x03, 0x61, 0x62, 0x63, 0x00}};
        std::string parsed_string;

        WHEN( "a client passes that buffer into decode_utf8_string" )
        {
            std::array<const char, 6>::iterator new_buffer_start;
            std::tie( new_buffer_start, parsed_string ) =
                decoder::decode_utf8_string( buffer.begin( ), buffer.cend( ) );

            THEN( "the client should receive an correctly parsed non-empty string" )
            {
                REQUIRE( parsed_string == "abc" );
            }

            THEN( "the client should receive a correctly updated buffer iterator" )
            {
                REQUIRE( new_buffer_start == buffer.begin( ) + 5 );
            }
        }
    }
}
