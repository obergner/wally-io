#include "catch.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <tuple>

#include <system_error>

#include "io_wally/codec/decoder.hpp"
#include "io_wally/error/protocol.hpp"
#include "io_wally/protocol/common.hpp"

using namespace io_wally;

SCENARIO( "frame_reader", "[packets]" )
{
    auto buffer = std::vector<uint8_t>( 512 );
    auto under_test = decoder::frame_reader{buffer};
    const auto ec_success = std::error_code{};

    GIVEN( "a well-formed packet header of length two encoding a remaining length of 0" )
    {
        const auto remaining_length = std::size_t{0};
        const auto type_and_flags = uint8_t{0x01 << 4};
        const auto serialized_header = std::array<uint8_t, 2>{{type_and_flags, 0x00}};
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor" )
        {
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            THEN( "it should receive 0" )
            {
                REQUIRE( frame_remainder == remaining_length );
            }

            AND_THEN( "it should receive a correctly decoded frame from get_frame()" )
            {
                const auto frame = under_test.get_frame( );

                REQUIRE( frame );
                REQUIRE( frame->type_and_flags == type_and_flags );
                REQUIRE( frame->remaining_length( ) == remaining_length );
            }
        }
    }

    GIVEN( "a well-formed packet header of length two encoding a remaining length of 127" )
    {
        const auto remaining_length = std::size_t{127};
        const auto type_and_flags = uint8_t{0x05 << 4};
        const auto serialized_header = std::array<uint8_t, 2>{{type_and_flags, 0x7F}};
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor for the first time" )
        {
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            THEN( "it should receive 127" )
            {
                REQUIRE( frame_remainder == remaining_length );
            }
        }

        WHEN( "a caller calls frame_reader as a functor when frame is completed" )
        {
            buffer.insert( std::end( buffer ), remaining_length, 0x00 );
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) + remaining_length );

            THEN( "it should receive 0" )
            {
                REQUIRE( frame_remainder == 0 );
            }

            AND_THEN( "it should receive a correctly decoded frame from get_frame()" )
            {
                const auto frame = under_test.get_frame( );

                REQUIRE( frame );
                REQUIRE( frame->type_and_flags == type_and_flags );
                REQUIRE( frame->remaining_length( ) == remaining_length );
                REQUIRE( *frame->begin == 0x00 );
                REQUIRE( *( frame->begin - 1 ) == 0x7F );
            }
        }
    }

    GIVEN( "a well-formed packet header of length three encoding a remaining length of 128" )
    {
        const auto remaining_length = std::size_t{128};
        const auto type_and_flags = uint8_t{0x07 << 4};
        const auto serialized_header = std::array<uint8_t, 3>{{type_and_flags, 0x80, 0x01}};
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor for the first time" )
        {
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            THEN( "it should receive 128" )
            {
                REQUIRE( frame_remainder == remaining_length );
            }
        }

        WHEN( "a caller calls frame_reader as a functor when frame is completed" )
        {
            buffer.insert( std::end( buffer ), remaining_length, 0x00 );
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) + remaining_length );

            THEN( "it should receive 0" )
            {
                REQUIRE( frame_remainder == 0 );
            }

            AND_THEN( "it should receive a correctly decoded frame from get_frame()" )
            {
                const auto frame = under_test.get_frame( );

                REQUIRE( frame );
                REQUIRE( frame->type_and_flags == type_and_flags );
                REQUIRE( frame->remaining_length( ) == remaining_length );
                REQUIRE( *frame->begin == 0x00 );
                REQUIRE( *( frame->begin - 1 ) == 0x01 );
            }
        }
    }

    GIVEN( "a well-formed packet header of length three encoding a remaining length of 127 * 128 + 127" )
    {
        const auto remaining_length = std::size_t{127 * 128 + 127};
        const auto type_and_flags = uint8_t{0x08 << 4};
        const auto serialized_header = std::array<uint8_t, 3>{{type_and_flags, 0xFF, 0x7F}};
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor for the first time" )
        {
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            THEN( "it should receive 127 * 128 + 127" )
            {
                REQUIRE( frame_remainder == remaining_length );
            }
        }

        WHEN( "a caller calls frame_reader as a functor when frame is completed" )
        {
            buffer.insert( std::end( buffer ), remaining_length, 0x00 );
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) + remaining_length );

            THEN( "it should receive 0" )
            {
                REQUIRE( frame_remainder == 0 );
            }

            AND_THEN( "it should receive a correctly decoded frame from get_frame()" )
            {
                const auto frame = under_test.get_frame( );

                REQUIRE( frame );
                REQUIRE( frame->type_and_flags == type_and_flags );
                REQUIRE( frame->remaining_length( ) == remaining_length );
                REQUIRE( *frame->begin == 0x00 );
                REQUIRE( *( frame->begin - 1 ) == 0x7F );
            }
        }
    }

    GIVEN( "a well-formed packet header of length four encoding a remaining length of 128 * 128" )
    {
        const auto remaining_length = std::size_t{128 * 128};
        const auto type_and_flags = uint8_t{0x09 << 4};
        const auto serialized_header = std::array<uint8_t, 4>{{type_and_flags, 0x80, 0x80, 0x01}};
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor for the first time" )
        {
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            THEN( "it should receive 128 * 128" )
            {
                REQUIRE( frame_remainder == remaining_length );
            }
        }

        WHEN( "a caller calls frame_reader as a functor when frame is completed" )
        {
            buffer.insert( std::end( buffer ), remaining_length, 0x00 );
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) + remaining_length );

            THEN( "it should receive 0" )
            {
                REQUIRE( frame_remainder == 0 );
            }

            AND_THEN( "it should receive a correctly decoded frame from get_frame()" )
            {
                const auto frame = under_test.get_frame( );

                REQUIRE( frame );
                REQUIRE( frame->type_and_flags == type_and_flags );
                REQUIRE( frame->remaining_length( ) == remaining_length );
                REQUIRE( *frame->begin == 0x00 );
                REQUIRE( *( frame->begin - 1 ) == 0x01 );
            }
        }
    }

    GIVEN(
        "a well-formed packet header of length four encoding a remaining length of 127 * 128 * 128 + 127 * 128 + "
        "127" )
    {
        const auto remaining_length = std::size_t{127 * 128 * 128 + 127 * 128 + 127};
        const auto type_and_flags = uint8_t{0x0A << 4};
        const auto serialized_header = std::array<uint8_t, 4>{{type_and_flags, 0xFF, 0xFF, 0x7F}};
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor for the first time" )
        {
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            THEN( "it should receive 127 * 128 * 128 + 127 * 128 + 127" )
            {
                REQUIRE( frame_remainder == remaining_length );
            }
        }

        WHEN( "a caller calls frame_reader as a functor when frame is completed" )
        {
            buffer.insert( std::end( buffer ), remaining_length, 0x00 );
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) + remaining_length );

            THEN( "it should receive 0" )
            {
                REQUIRE( frame_remainder == 0 );
            }

            AND_THEN( "it should receive a correctly decoded frame from get_frame()" )
            {
                const auto frame = under_test.get_frame( );

                REQUIRE( frame );
                REQUIRE( frame->type_and_flags == type_and_flags );
                REQUIRE( frame->remaining_length( ) == remaining_length );
                REQUIRE( *frame->begin == 0x00 );
                REQUIRE( *( frame->begin - 1 ) == 0x7F );
            }
        }
    }

    GIVEN( "a well-formed packet header of length five encoding a remaining length of 128 * 128 * 128" )
    {
        const auto remaining_length = std::size_t{128 * 128 * 128};
        const auto type_and_flags = uint8_t{0x0B << 4};
        const auto serialized_header = std::array<uint8_t, 5>{{type_and_flags, 0x80, 0x80, 0x80, 0x01}};
        buffer.insert( std::begin( buffer ), std::begin( serialized_header ), std::end( serialized_header ) );

        WHEN( "a caller calls frame_reader as a functor for the first time" )
        {
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) );

            THEN( "it should receive 128 * 128 * 128" )
            {
                REQUIRE( frame_remainder == remaining_length );
            }
        }

        WHEN( "a caller calls frame_reader as a functor when frame is completed" )
        {
            buffer.insert( std::end( buffer ), remaining_length, 0x00 );
            const auto frame_remainder = under_test( ec_success, serialized_header.size( ) + remaining_length );

            THEN( "it should receive 0" )
            {
                REQUIRE( frame_remainder == 0 );
            }

            AND_THEN( "it should receive a correctly decoded frame from get_frame()" )
            {
                const auto frame = under_test.get_frame( );

                REQUIRE( frame );
                REQUIRE( frame->type_and_flags == type_and_flags );
                REQUIRE( frame->remaining_length( ) == remaining_length );
                REQUIRE( *frame->begin == 0x00 );
                REQUIRE( *( frame->begin - 1 ) == 0x01 );
            }
        }
    }

    GIVEN( "a well-formed packet header of length five encoding maximum allowed remaining length" )
    {
        const auto type_and_flags = uint8_t{0x0C << 4};
        const auto serialized_header = std::array<uint8_t, 5>{{type_and_flags, 0xFF, 0xFF, 0xFF, 0x7F}};
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

                    if ( frame_remainder > 0 )
                    {
                        REQUIRE( !under_test.get_frame( ) );
                    }
                }

                REQUIRE( under_test( ec_success, bytes_transferred ) == 0 );
                REQUIRE( under_test.get_frame( ) );
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

    GIVEN( "a partial packet header of eventual length 4" )
    {
        const auto serialized_header = std::array<uint8_t, 2>{{0x07 << 4, 0xFF}};
        const auto ec_success = std::error_code{};
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

        WHEN( "a caller repeatedly calls frame_reader as a functor" )
        {
            THEN( "it should receive std::nullopt from get_frame() while frame is incomplete" )
            {
                under_test( ec_success, bytes_transferred );
                REQUIRE( !under_test.get_frame( ) );

                buffer.insert( std::begin( buffer ) + bytes_transferred++, 0xFF );
                under_test( ec_success, bytes_transferred );
                REQUIRE( !under_test.get_frame( ) );

                buffer.insert( std::begin( buffer ) + bytes_transferred++, 0x7F );
                under_test( ec_success, bytes_transferred );
                REQUIRE( !under_test.get_frame( ) );
            }
        }
    }

    GIVEN( "a well-formed PUBLISH frame with QoS 1 (suspected bug)" )
    {
        const auto remaining_length = std::size_t{0x27};
        const auto type_and_flags = uint8_t{0x32};
        auto const received_packet = std::vector<std::uint8_t>{
            0x32,  // Type and flags
            0x27,  // Remaining length
            0x00,  // topic name MSB (0)
            0x12,  // topic name LSB (18)
            '/',  't', 'e', 's', 't', '/', 'p', 'u', 'b', 'l', 'i', 's', 'h', '/', 'q', 'o', 's', '1',
            0x00,  // packet ID MSB (0)
            0x01,  // packet ID LSB (1)
            't',  'e', 's', 't', '_', 'p', 'u', 'b', 'l', 'i', 's', 'h', '_', 'q', 'o', 's', '1',
        };

        WHEN( "a caller calls frame_reader as a functor when frame is completed" )
        {
            buffer.insert( std::begin( buffer ), std::begin( received_packet ), std::end( received_packet ) );
            const auto frame_remainder = under_test( ec_success, 2 + remaining_length );

            THEN( "it should receive 0" )
            {
                REQUIRE( frame_remainder == 0 );
            }

            AND_THEN( "it should receive a correctly decoded frame from get_frame()" )
            {
                const auto frame = under_test.get_frame( );

                REQUIRE( frame );
                REQUIRE( frame->type_and_flags == type_and_flags );
                REQUIRE( frame->remaining_length( ) == remaining_length );
                REQUIRE( *frame->begin == 0x00 );
                REQUIRE( *( frame->begin - 1 ) == 0x27 );
                REQUIRE( *( frame->end - 1 ) == '1' );
            }
        }
    }
}

SCENARIO( "parsing a 16 bit unsigned integer", "[packets]" )
{

    GIVEN( "a buffer of length 0" )
    {
        const auto buffer = std::array<const std::uint8_t, 0>{{}};

        WHEN( "a client passes that buffer into parse_std::uint16" )
        {
            THEN( "the client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( decoder::decode_uint16( buffer.begin( ), buffer.cend( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a buffer of length 1" )
    {
        const auto buffer = std::array<const std::uint8_t, 1>{{0x00}};

        WHEN( "a client passes that buffer into parse_std::uint16" )
        {
            THEN( "the client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( decoder::decode_uint16( buffer.begin( ), buffer.cend( ) ),
                                   error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a buffer of length 2" )
    {
        const auto msb = std::uint8_t{0x08};
        const auto lsb = std::uint8_t{0xEA};
        const auto buffer = std::array<const std::uint8_t, 2>{{msb, lsb}};
        auto parsed_int = std::uint16_t{0};

        const auto expected_result = std::uint16_t{( msb << 8 ) + lsb};

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
        const auto buffer = std::array<const char, 1>{{0x00}};

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
        const auto buffer = std::array<const char, 5>{{0x00, 0x04, 0x61, 0x62, 0x63}};

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
        const auto buffer = std::array<const char, 3>{{0x00, 0x00, 0x00}};
        auto parsed_string = std::string{};

        WHEN( "a client passes that buffer into decode_utf8_string" )
        {
            auto new_buffer_start = std::array<const char, 3>::iterator{};
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
        const auto buffer = std::array<char, 6>{{0x00, 0x03, 0x61, 0x62, 0x63, 0x00}};
        auto parsed_string = std::string{};

        WHEN( "a client passes that buffer into decode_utf8_string" )
        {
            auto new_buffer_start = std::array<const char, 6>::iterator{};
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

SCENARIO( "parsing a QoS byte", "[packets]" )
{

    GIVEN( "a buffer of length 0" )
    {
        const auto buffer = std::array<const std::uint8_t, 0>{{}};
        auto qos = protocol::packet::QoS{};

        WHEN( "a client passes that buffer into decode_qos" )
        {
            THEN( "the client should see an error::malformed_mqtt_packet being thrown" )
            {
                REQUIRE_THROWS_AS( decoder::decode_qos( buffer.begin( ), buffer.cend( ), &qos ),
                                   error::malformed_mqtt_packet );
            }
        }
    }

    GIVEN( "a buffer of length 1 containing QoS::AT_MOST_ONCE" )
    {
        const auto buffer = std::array<const std::uint8_t, 1>{{0x00}};
        auto qos = protocol::packet::QoS{};

        WHEN( "a client passes that buffer into decode_qos" )
        {
            const auto new_buffer_start = decoder::decode_qos( buffer.begin( ), buffer.cend( ), &qos );

            THEN( "the client should receive a correctly decoded QoS byte" )
            {
                REQUIRE( qos == protocol::packet::QoS::AT_MOST_ONCE );
            }

            THEN( "the client should receive a correctly updated buffer iterator" )
            {
                REQUIRE( new_buffer_start == buffer.begin( ) + 1 );
            }
        }
    }

    GIVEN( "a buffer of length 1 containing QoS::AT_LEAST_ONCE" )
    {
        const auto buffer = std::array<const std::uint8_t, 1>{{0x01}};
        auto qos = protocol::packet::QoS{};

        WHEN( "a client passes that buffer into decode_qos" )
        {
            const auto new_buffer_start = decoder::decode_qos( buffer.begin( ), buffer.cend( ), &qos );

            THEN( "the client should receive a correctly decoded QoS byte" )
            {
                REQUIRE( qos == protocol::packet::QoS::AT_LEAST_ONCE );
            }

            THEN( "the client should receive a correctly updated buffer iterator" )
            {
                REQUIRE( new_buffer_start == buffer.begin( ) + 1 );
            }
        }
    }

    GIVEN( "a buffer of length 1 containing QoS::EXACTLY_ONCE" )
    {
        const auto buffer = std::array<const std::uint8_t, 1>{{0x02}};
        auto qos = protocol::packet::QoS{};

        WHEN( "a client passes that buffer into decode_qos" )
        {
            const auto new_buffer_start = decoder::decode_qos( buffer.begin( ), buffer.cend( ), &qos );

            THEN( "the client should receive a correctly decoded QoS byte" )
            {
                REQUIRE( qos == protocol::packet::QoS::EXACTLY_ONCE );
            }

            THEN( "the client should receive a correctly updated buffer iterator" )
            {
                REQUIRE( new_buffer_start == buffer.begin( ) + 1 );
            }
        }
    }

    GIVEN( "a buffer of length 1 containing none of QoS::AT_MOST_ONCE, QoS::AT_LEAST_ONCE,  QoS::EXACTLY_ONCE" )
    {
        const auto buffer = std::array<const std::uint8_t, 1>{{0x03}};
        auto qos = protocol::packet::QoS{};

        WHEN( "a client passes that buffer into decode_qos" )
        {
            const auto new_buffer_start = decoder::decode_qos( buffer.begin( ), buffer.cend( ), &qos );

            THEN( "the client should receive a correctly decoded QoS::RESERVED byte" )
            {
                REQUIRE( qos == protocol::packet::QoS::RESERVED );
            }

            THEN( "the client should receive a correctly updated buffer iterator" )
            {
                REQUIRE( new_buffer_start == buffer.begin( ) + 1 );
            }
        }
    }
}
