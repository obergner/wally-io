#include "catch.hpp"

#include "io_wally/codec/encoder.hpp"

using namespace io_wally;

SCENARIO( "remaining_length_encoder", "[packets]" )
{
    typedef std::array<uint8_t, 4>::iterator itr;
    encoder::remaining_length_encoder<itr> under_test;

    GIVEN( "a valid remaining length 0x6E" )
    {
        const uint32_t remaining_length = 0x6E;
        std::array<uint8_t, 4> buf = {{0x00, 0x00, 0x00, 0x00}};  // double braces avoid warnings
        const std::array<uint8_t, 4> expected_result = {{remaining_length, 0x00, 0x00, 0x00}};

        WHEN( "a caller passes in that length" )
        {
            itr out_iter = under_test.encode( remaining_length, buf.begin( ) );

            THEN( "it should receive a correctly encoded remaining length and a correct out iterator" )
            {
                CHECK( buf == expected_result );
                REQUIRE( ( out_iter - buf.data( ) ) == 1 );
            }
        }
    }

    GIVEN( "a valid remaining length 0x80" )
    {
        const uint32_t remaining_length = 0x80;
        std::array<uint8_t, 4> buf = {{0x00, 0x00, 0x00, 0x00}};  // double braces avoid warnings
        const std::array<uint8_t, 4> expected_result = {{0x80, 0x01, 0x00, 0x00}};

        WHEN( "a caller passes in that length" )
        {
            itr out_iter = under_test.encode( remaining_length, buf.begin( ) );

            THEN( "it should receive a correctly encoded remaining length and a correct out iterator" )
            {
                CHECK( buf == expected_result );
                REQUIRE( ( out_iter - buf.data( ) ) == 2 );
            }
        }
    }

    GIVEN( "a valid remaining length 0x80 + 0x4F" )
    {
        const uint32_t remaining_length = 0x80 + 0x4F;
        std::array<uint8_t, 4> buf = {{0x00, 0x00, 0x00, 0x00}};  // double braces avoid warnings
        const std::array<uint8_t, 4> expected_result = {{0xCF, 0x01, 0x00, 0x00}};

        WHEN( "a caller passes in that length" )
        {
            itr out_iter = under_test.encode( remaining_length, buf.begin( ) );

            THEN( "it should receive a correctly encoded remaining length and a correct out iterator" )
            {
                CHECK( buf == expected_result );
                REQUIRE( ( out_iter - buf.data( ) ) == 2 );
            }
        }
    }

    GIVEN( "a valid remaining length 0x3D * (0x80 * 0x80) + 0x80 + 0x11" )
    {
        const uint32_t remaining_length = 0x3D * ( 0x80 * 0x80 ) + 0x80 + 0x11;
        std::array<uint8_t, 4> buf = {{0x00, 0x00, 0x00, 0x00}};  // double braces avoid warnings
        const std::array<uint8_t, 4> expected_result = {{0x91, 0x81, 0x3D, 0x00}};

        WHEN( "a caller passes in that length" )
        {
            itr out_iter = under_test.encode( remaining_length, buf.begin( ) );

            THEN( "it should receive a correctly encoded remaining length and a correct out iterator" )
            {
                CHECK( buf == expected_result );
                REQUIRE( ( out_iter - buf.data( ) ) == 3 );
            }
        }
    }

    GIVEN( "a valid remaining length 0x66 * (0x80 * 0x80 * 0x80) * 0x23 * (0x80 * 0x80) + 0x5F * 0x80 + 0x33" )
    {
        const uint32_t remaining_length = 0x22 * ( 0x80 * 0x80 * 0x80 ) + 0x11 * ( 0x80 * 0x80 ) + 0x5F * 0x80 + 0x33;
        std::array<uint8_t, 4> buf = {{0x00, 0x00, 0x00, 0x00}};  // double braces avoid warnings
        const std::array<uint8_t, 4> expected_result = {{0xB3, 0xDF, 0x91, 0x22}};

        WHEN( "a caller passes in that length" )
        {
            itr out_iter = under_test.encode( remaining_length, buf.begin( ) );

            THEN( "it should receive a correctly encoded remaining length and a correct out iterator" )
            {
                CHECK( buf == expected_result );
                REQUIRE( ( out_iter - buf.data( ) ) == 4 );
            }
        }
    }

    GIVEN( "the maximum allowed remaining length of 268,435,455" )
    {
        const uint32_t remaining_length = 268435455;
        std::array<uint8_t, 4> buf = {{0x00, 0x00, 0x00, 0x00}};  // double braces avoid warnings
        const std::array<uint8_t, 4> expected_result = {{0xFF, 0xFF, 0xFF, 0x7F}};

        WHEN( "a caller passes in that length" )
        {
            itr out_iter = under_test.encode( remaining_length, buf.begin( ) );

            THEN( "it should receive a correctly encoded remaining length and a correct out iterator" )
            {
                CHECK( buf == expected_result );
                REQUIRE( ( out_iter - buf.data( ) ) == 4 );
            }
        }
    }

    GIVEN( "an illegal remaining length of 268,435,455 + 1" )
    {
        const uint32_t remaining_length = 268435455 + 1;
        std::array<uint8_t, 4> buf = {{0x00, 0x00, 0x00, 0x00}};  // double braces avoid warnings

        WHEN( "a caller passes in that length" )
        {
            THEN( "it should see an encoder::error::illegal_mqtt_packet" )
            {
                REQUIRE_THROWS_AS( under_test.encode( remaining_length, buf.begin( ) ),
                                   encoder::error::illegal_mqtt_packet );
            }
        }
    }
}

SCENARIO( "encoding a 16 bit unsigned integer", "[packets]" )
{
    typedef std::array<const uint8_t, 2>::iterator out_iter;

    GIVEN( "the value 0" )
    {
        const uint16_t value = 0;
        std::array<uint8_t, 2> result = {{0x00, 0x00}};
        const std::array<uint8_t, 2> expected_result = {{0x00, 0x00}};

        WHEN( "a client passes that buffer into encode_uint16" )
        {
            const out_iter buf_end = encoder::encode_uint16( value, result.begin( ) );

            THEN( "the client should see a correctly encoded value and a correctly updated buffer iterator" )
            {
                CHECK( result == expected_result );
                REQUIRE( ( buf_end - result.begin( ) ) == 2 );
            }
        }
    }

    GIVEN( "the value 0x00F0" )
    {
        const uint16_t value = 0x00F0;
        std::array<uint8_t, 2> result = {{0x00, 0x00}};
        const std::array<uint8_t, 2> expected_result = {{0x00, 0xF0}};

        WHEN( "a client passes that buffer into encode_uint16" )
        {
            const out_iter buf_end = encoder::encode_uint16( value, result.begin( ) );

            THEN( "the client should see a correctly encoded value and a correctly updated buffer iterator" )
            {
                CHECK( result == expected_result );
                REQUIRE( ( buf_end - result.begin( ) ) == 2 );
            }
        }
    }

    GIVEN( "the value 0xB345" )
    {
        const uint16_t value = 0xB345;
        std::array<uint8_t, 2> result = {{0x00, 0x00}};
        const std::array<uint8_t, 2> expected_result = {{0xB3, 0x45}};

        WHEN( "a client passes that buffer into encode_uint16" )
        {
            const out_iter buf_end = encoder::encode_uint16( value, result.begin( ) );

            THEN( "the client should see a correctly encoded value and a correctly updated buffer iterator" )
            {
                for ( const uint8_t& v : result )
                    std::cout << "RES: " << (int)v << std::endl;
                std::cout.flush( );
                CHECK( result == expected_result );
                REQUIRE( ( buf_end - result.begin( ) ) == 2 );
            }
        }
    }
}
