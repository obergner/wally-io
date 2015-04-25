#include "catch.hpp"

#include "mqtt/protocol/packets.hpp"

using namespace mqtt::protocol;

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

SCENARIO( "connect_header", "[packets]" )
{

    GIVEN( "a connect_header with default protocol name" )
    {
        const char* const prot_name = "MQTT";
        const uint8_t prot_level = 0x04;
        const uint8_t prot_flags = 0x00;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks for the protocol name" )
        {
            const char* const name = under_test.protocol_name( );

            THEN( "it should see 'MQTT'" )
            {
                REQUIRE( name == prot_name );
            }
        }
    }

    GIVEN( "a connect_header with protocol name 'Custom protocol'" )
    {
        const char* const prot_name = "Custom protocol";
        const uint8_t prot_level = 0x04;
        const uint8_t prot_flags = 0x00;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks for the protocol name" )
        {
            const char* const name = under_test.protocol_name( );

            THEN( "it should see 'Custom protocol'" )
            {
                REQUIRE( name == prot_name );
            }
        }
    }

    GIVEN( "a connect_header with default protocol level" )
    {
        const char* const prot_name = "MQTT";
        const uint8_t prot_level = 0x04;
        const uint8_t prot_flags = 0x00;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks for the protocol level" )
        {
            const packet::protocol_level level = under_test.protocol_level( );

            THEN( "it should see level 4" )
            {
                REQUIRE( level == packet::LEVEL4 );
            }
        }
    }

    GIVEN( "a connect_header with non-default protocol level" )
    {
        const char* const prot_name = "MQTT";
        const uint8_t prot_level = 0x03;
        const uint8_t prot_flags = 0x00;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks for the protocol level" )
        {
            const packet::protocol_level level = under_test.protocol_level( );

            THEN( "it should see level UNSUPPORTED" )
            {
                REQUIRE( level == packet::UNSUPPORTED );
            }
        }
    }

    GIVEN( "a connect_header with username flag set" )
    {
        const char* const prot_name = "MQTT";
        const uint8_t prot_level = 0x04;
        const uint8_t prot_flags = 0x80;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks if the username flag is set" )
        {
            const bool has_username = under_test.has_username( );

            THEN( "it should receive answer 'true'" )
            {
                REQUIRE( has_username == true );
            }
        }
    }

    GIVEN( "a connect_header with username flag not set" )
    {
        const char* const prot_name = "MQTT";
        const uint8_t prot_level = 0x04;
        const uint8_t prot_flags = 0x7F;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks if the username flag is set" )
        {
            const bool has_username = under_test.has_username( );

            THEN( "it should receive answer 'false'" )
            {
                REQUIRE( has_username == false );
            }
        }
    }

    GIVEN( "a connect_header with password flag set" )
    {
        const char* const prot_name = "MQTT";
        const uint8_t prot_level = 0x04;
        const uint8_t prot_flags = 0x40;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks if the password flag is set" )
        {
            const bool has_password = under_test.has_password( );

            THEN( "it should receive answer 'true'" )
            {
                REQUIRE( has_password == true );
            }
        }
    }

    GIVEN( "a connect_header with password flag not set" )
    {
        const char* const prot_name = "MQTT";
        const uint8_t prot_level = 0x04;
        const uint8_t prot_flags = 0xBF;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks if the password flag is set" )
        {
            const bool has_password = under_test.has_password( );

            THEN( "it should receive answer 'false'" )
            {
                REQUIRE( has_password == false );
            }
        }
    }

    GIVEN( "a connect_header with last will retain flag set" )
    {
        const char* const prot_name = "MQTT";
        const uint8_t prot_level = 0x04;
        const uint8_t prot_flags = 0x20;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks if the receiver should retain any last will message" )
        {
            const bool must_retain_last_will = under_test.retain_last_will( );

            THEN( "it should receive answer 'true'" )
            {
                REQUIRE( must_retain_last_will == true );
            }
        }
    }

    GIVEN( "a connect_header with last will retain flag not set" )
    {
        const char* const prot_name = "MQTT";
        const uint8_t prot_level = 0x04;
        const uint8_t prot_flags = 0xDF;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks if the receiver should retain any last will message" )
        {
            const bool must_retain_last_will = under_test.retain_last_will( );

            THEN( "it should receive answer 'false'" )
            {
                REQUIRE( must_retain_last_will == false );
            }
        }
    }

    GIVEN( "a connect_header with last will quality of service 'Exactly Once'" )
    {
        const char* const prot_name = "MQTT";
        const uint8_t prot_level = 0x04;
        const uint8_t prot_flags = 0x10;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks for the last will quality of service" )
        {
            const packet::qos qos = under_test.last_will_qos( );

            THEN( "it should receive answer 'EXACTLY_ONCE'" )
            {
                REQUIRE( qos == packet::EXACTLY_ONCE );
            }
        }
    }

    GIVEN( "a connect_header with last will quality of service 'At least once'" )
    {
        const char* const prot_name = "MQTT";
        const uint8_t prot_level = 0x04;
        const uint8_t prot_flags = 0x08;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks for the last will quality of service" )
        {
            const packet::qos qos = under_test.last_will_qos( );

            THEN( "it should receive answer 'AT_LEAST_ONCE'" )
            {
                REQUIRE( qos == packet::AT_LEAST_ONCE );
            }
        }
    }

    GIVEN( "a connect_header with last will quality of service 'At most once'" )
    {
        const char* const prot_name = "MQTT";
        const uint8_t prot_level = 0x04;
        const uint8_t prot_flags = 0xE7;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks for the last will quality of service" )
        {
            const packet::qos qos = under_test.last_will_qos( );

            THEN( "it should receive answer 'AT_MOST_ONCE'" )
            {
                REQUIRE( qos == packet::AT_MOST_ONCE );
            }
        }
    }

    GIVEN( "a connect_header with the last will flag set" )
    {
        const char* const prot_name = "MQTT";
        const uint8_t prot_level = 0x04;
        const uint8_t prot_flags = 0x04;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks whether the packet contains a last will message" )
        {
            const bool retain_last_will = under_test.contains_last_will( );

            THEN( "it should receive answer 'true'" )
            {
                REQUIRE( retain_last_will == true );
            }
        }
    }

    GIVEN( "a connect_header with the last will flag not set" )
    {
        const char* const prot_name = "MQTT";
        const uint8_t prot_level = 0x04;
        const uint8_t prot_flags = 0xFB;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks whether the packet contains a last will message" )
        {
            const bool retain_last_will = under_test.contains_last_will( );

            THEN( "it should receive answer 'false'" )
            {
                REQUIRE( retain_last_will == false );
            }
        }
    }

    GIVEN( "a connect_header with the clean session flag set" )
    {
        const char* const prot_name = "MQTT";
        const uint8_t prot_level = 0x04;
        const uint8_t prot_flags = 0x02;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks whether the client session should be cleaned" )
        {
            const bool clean_session = under_test.clean_session( );

            THEN( "it should receive answer 'true'" )
            {
                REQUIRE( clean_session == true );
            }
        }
    }

    GIVEN( "a connect_header with the clean session flag not set" )
    {
        const char* const prot_name = "MQTT";
        const uint8_t prot_level = 0x04;
        const uint8_t prot_flags = 0xFD;
        const uint16_t keep_alive_secs = 0x0000;
        const connect_header under_test( prot_name, prot_level, prot_flags, keep_alive_secs );

        WHEN( "a caller asks whether the client session should be cleaned" )
        {
            const bool clean_session = under_test.clean_session( );

            THEN( "it should receive answer 'false'" )
            {
                REQUIRE( clean_session == false );
            }
        }
    }
}
