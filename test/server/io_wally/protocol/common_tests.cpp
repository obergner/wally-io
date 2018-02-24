#include "catch.hpp"

#include <cstdint>

#include "io_wally/protocol/common.hpp"

using namespace io_wally::protocol;

TEST_CASE( "MQTT header flags", "[header]" )
{
    SECTION( "all flags are set" )
    {
        const auto flgs = uint8_t{0x0F};

        REQUIRE( packet::dup_of( flgs ) );
        REQUIRE( packet::retain_of( flgs ) );
        REQUIRE( packet::qos_of( flgs, 1 ) == packet::QoS::RESERVED );
    }

    SECTION( "only flags 0 and 1 are set" )
    {
        const auto flgs = uint8_t{0x03};

        REQUIRE( !packet::dup_of( flgs ) );
        REQUIRE( packet::retain_of( flgs ) );
        REQUIRE( packet::qos_of( flgs, 1 ) == packet::QoS::AT_LEAST_ONCE );
    }

    SECTION( "only flags 1 and 2 are set" )
    {
        const auto flgs = uint8_t{0x06};

        REQUIRE( !packet::dup_of( flgs ) );
        REQUIRE( !packet::retain_of( flgs ) );
        REQUIRE( packet::qos_of( flgs, 1 ) == packet::QoS::RESERVED );
    }

    SECTION( "only flags 2 and 3 are set" )
    {
        const auto flgs = uint8_t{0x0C};

        REQUIRE( packet::dup_of( flgs ) );
        REQUIRE( !packet::retain_of( flgs ) );
        REQUIRE( packet::qos_of( flgs, 1 ) == packet::QoS::EXACTLY_ONCE );
    }
}

SCENARIO( "Converting a byte into QoS", "[packets]" )
{
    GIVEN( "a byte encoding QoS::AT_MOST_ONCE in bits 0 and 1 with some other bits set" )
    {
        const auto input = std::uint8_t{0x0C};

        WHEN( "a caller passes that byte into qos_of()" )
        {
            const auto qos = packet::qos_of( input );

            THEN( "it will receive QoS::AT_MOST_ONCE" )
            {
                REQUIRE( qos == packet::QoS::AT_MOST_ONCE );
            }
        }
    }

    GIVEN( "a byte encoding QoS::AT_LEAST_ONCE in bits 0 and 1 with some other bits set" )
    {
        const auto input = std::uint8_t{0x1D};

        WHEN( "a caller passes that byte into qos_of()" )
        {
            const auto qos = packet::qos_of( input );

            THEN( "it will receive QoS::AT_LEAST_ONCE" )
            {
                REQUIRE( qos == packet::QoS::AT_LEAST_ONCE );
            }
        }
    }

    GIVEN( "a byte encoding QoS::EXACTLY_ONCE in bits 0 and 1 with some other bits set" )
    {
        const auto input = std::uint8_t{0x1E};

        WHEN( "a caller passes that byte into qos_of()" )
        {
            const auto qos = packet::qos_of( input );

            THEN( "it will receive QoS::EXACTLY_ONCE" )
            {
                REQUIRE( qos == packet::QoS::EXACTLY_ONCE );
            }
        }
    }

    GIVEN( "a byte encoding QoS::AT_LEAST_ONCE in bits 2 and 3 with some other bits set" )
    {
        const auto input = std::uint8_t{0x15};

        WHEN( "a caller passes that byte into qos_of() with left_shift 2" )
        {
            const auto qos = packet::qos_of( input, 2 );

            THEN( "it will receive QoS::AT_LEAST_ONCE" )
            {
                REQUIRE( qos == packet::QoS::AT_LEAST_ONCE );
            }
        }
    }

    GIVEN( "a byte encoding QoS::EXACTLY_ONCE in bits 6 and 7 with some other bits set" )
    {
        const auto input = std::uint8_t{0x88};

        WHEN( "a caller passes that byte into qos_of() with left_shift 6" )
        {
            const auto qos = packet::qos_of( input, 6 );

            THEN( "it will receive QoS::EXACTLY_ONCE" )
            {
                REQUIRE( qos == packet::QoS::EXACTLY_ONCE );
            }
        }
    }
}

SCENARIO( "writing a QoS into a byte", "[packets]" )
{
    GIVEN( "a byte with some bits set" )
    {
        auto byte = std::uint8_t{0x0F};
        const auto expected_byte = std::uint8_t{0x0C};

        WHEN( "a caller passes QoS::AT_MOST_ONCE and the given byte into qos_into() with left_shift 0" )
        {
            packet::qos_into( packet::QoS::AT_MOST_ONCE, byte );

            THEN( "it will see a correctly updated byte" )
            {
                REQUIRE( byte == expected_byte );
            }
        }
    }

    GIVEN( "a byte with some bits set" )
    {
        auto byte = std::uint8_t{0x0F};
        const auto expected_byte = std::uint8_t{0x0D};

        WHEN( "a caller passes QoS::AT_LEAST_ONCE and the given byte into qos_into() with left_shift 0" )
        {
            packet::qos_into( packet::QoS::AT_LEAST_ONCE, byte );

            THEN( "it will see a correctly updated byte" )
            {
                REQUIRE( byte == expected_byte );
            }
        }
    }

    GIVEN( "a byte with some bits set" )
    {
        auto byte = std::uint8_t{0x0F};
        const auto expected_byte = std::uint8_t{0x0E};

        WHEN( "a caller passes QoS::EXACTLY_ONCE and the given byte into qos_into() with left_shift 0" )
        {
            packet::qos_into( packet::QoS::EXACTLY_ONCE, byte );

            THEN( "it will see a correctly updated byte" )
            {
                REQUIRE( byte == expected_byte );
            }
        }
    }

    GIVEN( "a byte with some bits set" )
    {
        auto byte = std::uint8_t{0xFF};
        const auto expected_byte = std::uint8_t{0xEF};

        WHEN( "a caller passes QoS::AT_LEAST_ONCE and the given byte into qos_into() with left_shift 3" )
        {
            packet::qos_into( packet::QoS::AT_LEAST_ONCE, byte, 3 );

            THEN( "it will see a correctly updated byte" )
            {
                REQUIRE( byte == expected_byte );
            }
        }
    }
}

SCENARIO( "converting a header byte into Type", "[packets]" )
{
    GIVEN( "a byte encoding type CONNECT with some header flags set" )
    {
        const auto input = uint8_t{( 0x01 << 4 ) | 0x09};

        WHEN( "a caller passes that byte into type_of()" )
        {
            const packet::Type type = packet::type_of( input );

            THEN( "it will receive Type::CONNECT" )
            {
                REQUIRE( type == packet::Type::CONNECT );
            }
        }
    }

    GIVEN( "a byte encoding type CONNACK with some header flags set" )
    {
        const auto input = uint8_t{( 0x02 << 4 ) | 0x0F};

        WHEN( "a caller passes that byte into type_of()" )
        {
            const packet::Type type = packet::type_of( input );

            THEN( "it will receive Type::CONNACK" )
            {
                REQUIRE( type == packet::Type::CONNACK );
            }
        }
    }

    GIVEN( "a byte encoding type PUBLISH with some header flags set" )
    {
        const auto input = uint8_t{( 0x03 << 4 ) | 0x0F};

        WHEN( "a caller passes that byte into type_of()" )
        {
            const packet::Type type = packet::type_of( input );

            THEN( "it will receive Type::PUBLISH" )
            {
                REQUIRE( type == packet::Type::PUBLISH );
            }
        }
    }

    GIVEN( "a byte encoding type PUBACK with some header flags set" )
    {
        const auto input = uint8_t{( 0x04 << 4 ) | 0x0F};

        WHEN( "a caller passes that byte into type_of()" )
        {
            const packet::Type type = packet::type_of( input );

            THEN( "it will receive Type::PUBACK" )
            {
                REQUIRE( type == packet::Type::PUBACK );
            }
        }
    }

    GIVEN( "a byte encoding type PUBREC with some header flags set" )
    {
        const auto input = uint8_t{( 0x05 << 4 ) | 0x0F};

        WHEN( "a caller passes that byte into type_of()" )
        {
            const packet::Type type = packet::type_of( input );

            THEN( "it will receive Type::PUBREC" )
            {
                REQUIRE( type == packet::Type::PUBREC );
            }
        }
    }

    GIVEN( "a byte encoding type PUBREL with some header flags set" )
    {
        const auto input = uint8_t{( 0x06 << 4 ) | 0x01};

        WHEN( "a caller passes that byte into type_of()" )
        {
            const packet::Type type = packet::type_of( input );

            THEN( "it will receive Type::PUBREL" )
            {
                REQUIRE( type == packet::Type::PUBREL );
            }
        }
    }

    GIVEN( "a byte encoding type PUBCOMP with some header flags set" )
    {
        const auto input = uint8_t{( 0x07 << 4 ) | 0x01};

        WHEN( "a caller passes that byte into type_of()" )
        {
            const packet::Type type = packet::type_of( input );

            THEN( "it will receive Type::PUBCOMP" )
            {
                REQUIRE( type == packet::Type::PUBCOMP );
            }
        }
    }

    GIVEN( "a byte encoding type SUBSCRIBE with some header flags set" )
    {
        const auto input = uint8_t{( 0x08 << 4 ) | 0x01};

        WHEN( "a caller passes that byte into type_of()" )
        {
            const packet::Type type = packet::type_of( input );

            THEN( "it will receive Type::SUBSCRIBE" )
            {
                REQUIRE( type == packet::Type::SUBSCRIBE );
            }
        }
    }

    GIVEN( "a byte encoding type SUBACK with some header flags set" )
    {
        const auto input = uint8_t{( 0x09 << 4 ) | 0x01};

        WHEN( "a caller passes that byte into type_of()" )
        {
            const packet::Type type = packet::type_of( input );

            THEN( "it will receive Type::SUBACK" )
            {
                REQUIRE( type == packet::Type::SUBACK );
            }
        }
    }

    GIVEN( "a byte encoding type UNSUBSCRIBE with some header flags set" )
    {
        const auto input = uint8_t{( 0x0A << 4 ) | 0x01};

        WHEN( "a caller passes that byte into type_of()" )
        {
            const packet::Type type = packet::type_of( input );

            THEN( "it will receive Type::UNSUBSCRIBE" )
            {
                REQUIRE( type == packet::Type::UNSUBSCRIBE );
            }
        }
    }

    GIVEN( "a byte encoding type UNSUBACK with some header flags set" )
    {
        const auto input = uint8_t{( 0x0B << 4 ) | 0x01};

        WHEN( "a caller passes that byte into type_of()" )
        {
            const packet::Type type = packet::type_of( input );

            THEN( "it will receive Type::UNSUBACK" )
            {
                REQUIRE( type == packet::Type::UNSUBACK );
            }
        }
    }

    GIVEN( "a byte encoding type PINGREQ with some header flags set" )
    {
        const auto input = uint8_t{( 0x0C << 4 ) | 0x01};

        WHEN( "a caller passes that byte into type_of()" )
        {
            const packet::Type type = packet::type_of( input );

            THEN( "it will receive Type::PINGREQ" )
            {
                REQUIRE( type == packet::Type::PINGREQ );
            }
        }
    }

    GIVEN( "a byte encoding type PINGRESP with some header flags set" )
    {
        const auto input = uint8_t{( 0x0D << 4 ) | 0x01};

        WHEN( "a caller passes that byte into type_of()" )
        {
            const packet::Type type = packet::type_of( input );

            THEN( "it will receive Type::PINGRESP" )
            {
                REQUIRE( type == packet::Type::PINGRESP );
            }
        }
    }

    GIVEN( "a byte encoding type DISCONNECT with some header flags set" )
    {
        const auto input = uint8_t{( 0x0E << 4 ) | 0x01};

        WHEN( "a caller passes that byte into type_of()" )
        {
            const packet::Type type = packet::type_of( input );

            THEN( "it will receive Type::DISCONNECT" )
            {
                REQUIRE( type == packet::Type::DISCONNECT );
            }
        }
    }
}

SCENARIO( "writing a Type into a header byte", "[packets]" )
{
    GIVEN( "a header byte with type CONNECT and some flags set" )
    {
        auto header_byte = uint8_t{( 0x01 << 4 ) | 0x0B};

        WHEN( "a caller passes type CONNACK and the given header byte into type_into()" )
        {
            const auto expected_header_byte = uint8_t{( 0x02 << 4 ) | 0x0B};

            packet::type_into( packet::Type::CONNACK, header_byte );

            THEN( "it will see a correctly updated header byte" )
            {
                REQUIRE( header_byte == expected_header_byte );
            }
        }

        WHEN( "a caller passes type PUBLISH and the given header byte into type_into()" )
        {
            const auto expected_header_byte = uint8_t{( 0x03 << 4 ) | 0x0B};

            packet::type_into( packet::Type::PUBLISH, header_byte );

            THEN( "it will see a correctly updated header byte" )
            {
                REQUIRE( header_byte == expected_header_byte );
            }
        }

        WHEN( "a caller passes type DISCONNECT and the given header byte into type_into()" )
        {
            const auto expected_header_byte = uint8_t{( 0x0E << 4 ) | 0x0B};

            packet::type_into( packet::Type::DISCONNECT, header_byte );

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
        const std::uint32_t remaining_length = 126;

        WHEN( "a caller calls total_length()" )
        {
            const std::uint32_t total_length = packet::total_length( remaining_length );

            THEN( "it will see a total length of remaining_length + 2 bytes" )
            {
                REQUIRE( total_length == remaining_length + 2 );
            }
        }
    }

    GIVEN( "a fixed header with remaining length 128" )
    {
        const std::uint32_t remaining_length = 128;

        WHEN( "a caller calls total_length()" )
        {
            const std::uint32_t total_length = packet::total_length( remaining_length );

            THEN( "it will see a total length of remaining_length + 3 bytes" )
            {
                REQUIRE( total_length == remaining_length + 3 );
            }
        }
    }

    GIVEN( "a fixed header with remaining length 16383" )
    {
        const std::uint32_t remaining_length = 16383;

        WHEN( "a caller calls total_length()" )
        {
            const std::uint32_t total_length = packet::total_length( remaining_length );

            THEN( "it will see a total length of remaining_length + 3 bytes" )
            {
                REQUIRE( total_length == remaining_length + 3 );
            }
        }
    }

    GIVEN( "a fixed header with remaining length 16384" )
    {
        const std::uint32_t remaining_length = 16384;

        WHEN( "a caller calls total_length()" )
        {
            const std::uint32_t total_length = packet::total_length( remaining_length );

            THEN( "it will see a total length of remaining_length + 4 bytes" )
            {
                REQUIRE( total_length == remaining_length + 4 );
            }
        }
    }

    GIVEN( "a fixed header with remaining length 2097151" )
    {
        const std::uint32_t remaining_length = 2097151;

        WHEN( "a caller calls total_length()" )
        {
            const std::uint32_t total_length = packet::total_length( remaining_length );

            THEN( "it will see a total length of remaining_length + 4 bytes" )
            {
                REQUIRE( total_length == remaining_length + 4 );
            }
        }
    }

    GIVEN( "a fixed header with remaining length 2097152" )
    {
        const std::uint32_t remaining_length = 2097152;

        WHEN( "a caller calls total_length()" )
        {
            const std::uint32_t total_length = packet::total_length( remaining_length );

            THEN( "it will see a total length of remaining_length + 5 bytes" )
            {
                REQUIRE( total_length == remaining_length + 5 );
            }
        }
    }

    GIVEN( "a fixed header with remaining length 268435455" )
    {
        const std::uint32_t remaining_length = 268435455;

        WHEN( "a caller calls total_length()" )
        {
            const std::uint32_t total_length = packet::total_length( remaining_length );

            THEN( "it will see a total length of remaining_length + 5 bytes" )
            {
                REQUIRE( total_length == remaining_length + 5 );
            }
        }
    }
}
