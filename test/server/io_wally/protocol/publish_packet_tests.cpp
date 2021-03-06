#include "catch.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include "io_wally/protocol/publish_packet.hpp"

using namespace std::string_literals;
using namespace io_wally::protocol;

SCENARIO( "publish", "[packets]" )
{
    GIVEN( "a publish with DUP set to false" )
    {
        const auto flgs = std::uint8_t{3 << 4 | 0x03};
        const auto rem_len = std::uint32_t{5};

        const auto topic = "a";

        const auto pktid = std::uint16_t{0xF312};

        const auto msg = std::vector<uint8_t>{0x00, 0x00};

        auto under_test = publish{flgs, rem_len, topic, pktid, msg};

        WHEN( "a caller calls dup( true )" )
        {
            under_test.dup( true );

            THEN( "dup() should return true" )
            {
                REQUIRE( under_test.dup( ) == true );
            }
        }

        WHEN( "a caller calls dup( false )" )
        {
            under_test.dup( false );

            THEN( "dup() should return false" )
            {
                REQUIRE( under_test.dup( ) == false );
            }
        }
    }

    GIVEN( "a publish with DUP set to true" )
    {
        const auto flgs = std::uint8_t{3 << 4 | 0x09};
        const auto rem_len = std::uint32_t{5};

        const auto topic = "a";

        const auto pktid = std::uint16_t{0xF312};

        const auto msg = std::vector<uint8_t>{0x00, 0x00};

        auto under_test = publish{flgs, rem_len, topic, pktid, msg};

        WHEN( "a caller calls dup( true )" )
        {
            under_test.dup( true );

            THEN( "dup() should return true" )
            {
                REQUIRE( under_test.dup( ) == true );
            }
        }

        WHEN( "a caller calls dup( false )" )
        {
            under_test.dup( false );

            THEN( "dup() should return false" )
            {
                REQUIRE( under_test.dup( ) == false );
            }
        }
    }

    GIVEN( "a publish with QoS At most once (QoS0)" )
    {
        const auto flgs = std::uint8_t{3 << 4 | 0x00};
        const auto rem_len = std::uint32_t{3};

        const auto topic = "a";

        const auto msg = std::vector<uint8_t>{0x01, 0x02};

        auto under_test = publish{flgs, rem_len, topic, 0, msg};

        WHEN( "a caller calls qos()" )
        {
            THEN( "it should see 'At most once' returned" )
            {
                CHECK( under_test.retain( ) == false );
                CHECK( under_test.dup( ) == false );
                CHECK( under_test.remaining_length( ) == rem_len );
                REQUIRE( under_test.qos( ) == packet::QoS::AT_MOST_ONCE );
            }
        }

        WHEN( "a caller sets 'At least once'" )
        {
            under_test.qos( packet::QoS::AT_LEAST_ONCE );
            THEN( "qos() should return 'At least once'" )
            {
                REQUIRE( under_test.qos( ) == packet::QoS::AT_LEAST_ONCE );
            }

            AND_THEN( "remaining_length() should be increased by two bytes" )
            {
                REQUIRE( under_test.remaining_length( ) == rem_len + 2 );
            }
        }

        WHEN( "a caller sets 'Exactly once'" )
        {
            under_test.qos( packet::QoS::EXACTLY_ONCE );
            THEN( "qos() should return 'Exactly once'" )
            {
                REQUIRE( under_test.qos( ) == packet::QoS::EXACTLY_ONCE );
            }

            AND_THEN( "remaining_length() should be increased by two bytes" )
            {
                REQUIRE( under_test.remaining_length( ) == rem_len + 2 );
            }
        }
    }

    GIVEN( "a publish with QoS At least once (QoS1)" )
    {
        const auto flgs = std::uint8_t{3 << 4 | 0x0B};
        const auto rem_len = std::uint32_t{5};

        const auto topic = "a";

        const auto pktid = std::uint16_t{0xF312};

        const auto msg = std::vector<uint8_t>{0x00, 0x00};

        auto under_test = publish{flgs, rem_len, topic, pktid, msg};

        WHEN( "a caller calls qos()" )
        {
            THEN( "it should see 'At least once' returned" )
            {
                CHECK( under_test.retain( ) == true );
                CHECK( under_test.dup( ) == true );
                REQUIRE( under_test.qos( ) == packet::QoS::AT_LEAST_ONCE );
            }
        }

        WHEN( "a caller sets 'Exactly once' and then calls qos()" )
        {
            under_test.qos( packet::QoS::EXACTLY_ONCE );
            THEN( "it should see 'Exactly once' returned" )
            {
                CHECK( under_test.retain( ) == true );
                CHECK( under_test.dup( ) == true );
                CHECK( under_test.remaining_length( ) == rem_len );
                REQUIRE( under_test.qos( ) == packet::QoS::EXACTLY_ONCE );
            }
        }

        WHEN( "a caller sets 'At most once'" )
        {
            under_test.qos( packet::QoS::AT_MOST_ONCE );
            THEN( "qos() should return 'At most once'" )
            {
                REQUIRE( under_test.qos( ) == packet::QoS::AT_MOST_ONCE );
            }

            AND_THEN( "remaining_length() should be decreased by two bytes" )
            {
                REQUIRE( under_test.remaining_length( ) == rem_len - 2 );
            }
        }
    }

    GIVEN( "a publish with QoS 'Exactly once' (QoS2)" )
    {
        const auto flgs = std::uint8_t{3 << 4 | 0x04};
        const auto rem_len = std::uint32_t{5};

        const auto topic = "a";

        const auto pktid = std::uint16_t{0xF312};

        const auto msg = std::vector<uint8_t>{0x00, 0x00};

        auto under_test = publish{flgs, rem_len, topic, pktid, msg};

        WHEN( "a caller calls qos()" )
        {
            THEN( "it should see 'Exactly once' returned" )
            {
                CHECK( under_test.retain( ) == false );
                CHECK( under_test.dup( ) == false );
                REQUIRE( under_test.qos( ) == packet::QoS::EXACTLY_ONCE );
            }
        }

        WHEN( "a caller sets 'At least once' and then calls qos()" )
        {
            under_test.qos( packet::QoS::AT_LEAST_ONCE );
            THEN( "it should see 'At least once' returned" )
            {
                CHECK( under_test.retain( ) == false );
                CHECK( under_test.dup( ) == false );
                CHECK( under_test.remaining_length( ) == rem_len );
                REQUIRE( under_test.qos( ) == packet::QoS::AT_LEAST_ONCE );
            }
        }

        WHEN( "a caller sets 'At most once'" )
        {
            under_test.qos( packet::QoS::AT_MOST_ONCE );
            THEN( "qos() should return 'At most once'" )
            {
                REQUIRE( under_test.qos( ) == packet::QoS::AT_MOST_ONCE );
            }

            AND_THEN( "remaining_length() should be decreased by two bytes" )
            {
                REQUIRE( under_test.remaining_length( ) == rem_len - 2 );
            }
        }
    }

    GIVEN( "a publish with RETAIN set to false" )
    {
        const auto flgs = std::uint8_t{3 << 4 | 0x02};
        const auto rem_len = std::uint32_t{5};

        const auto topic = "a";

        const auto pktid = std::uint16_t{0xF312};

        const auto msg = std::vector<uint8_t>{0x00, 0x00};

        auto under_test = publish{flgs, rem_len, topic, pktid, msg};

        WHEN( "a caller calls retain( false )" )
        {
            under_test.retain( false );

            THEN( "retain() should return false" )
            {
                REQUIRE( under_test.retain( ) == false );
            }
        }

        WHEN( "a caller calls retain( true )" )
        {
            under_test.retain( true );

            THEN( "retain() should return true" )
            {
                REQUIRE( under_test.retain( ) == true );
            }
        }
    }

    GIVEN( "a publish with RETAIN set to true" )
    {
        const auto flgs = std::uint8_t{3 << 4 | 0x03};
        const auto rem_len = std::uint32_t{5};

        const auto topic = "a";

        const auto pktid = std::uint16_t{0xF312};

        const auto msg = std::vector<uint8_t>{0x00, 0x00};

        auto under_test = publish{flgs, rem_len, topic, pktid, msg};

        WHEN( "a caller calls retain( false )" )
        {
            under_test.retain( false );

            THEN( "retain() should return false" )
            {
                REQUIRE( under_test.retain( ) == false );
            }
        }

        WHEN( "a caller calls retain( true )" )
        {
            under_test.retain( true );

            THEN( "retain() should return true" )
            {
                REQUIRE( under_test.retain( ) == true );
            }
        }
    }
}

SCENARIO( "publish#create", "[packets]" )
{
    GIVEN(
        "publish packet properties dup, retain, qos, topic, packet identifier and an application message of length 0" )
    {
        const auto dup = false;
        const auto qos = packet::QoS::AT_LEAST_ONCE;
        const auto retain = true;
        const auto topic = "just/a/test/topic"s;
        const auto pktid = std::uint16_t{0xF312};
        const auto msg = std::vector<uint8_t>{};

        const auto expected_remaining_length = 2 + ( 2 + topic.length( ) ) + msg.size( );

        WHEN( "a caller calls publish::create(...)" )
        {
            const auto publish_ptr = publish::create( dup, qos, retain, topic, pktid, msg );

            THEN( "it should receive a properly constructed publish packet" )
            {
                CHECK( publish_ptr->dup( ) == dup );
                CHECK( publish_ptr->qos( ) == qos );
                CHECK( publish_ptr->retain( ) == retain );
                CHECK( publish_ptr->topic( ) == topic );
                CHECK( publish_ptr->packet_identifier( ) == pktid );
                CHECK( publish_ptr->remaining_length( ) == expected_remaining_length );
                REQUIRE( publish_ptr->application_message( ) == msg );
            }
        }
    }

    GIVEN(
        "publish packet properties dup, retain, qos, topic, packet identifier and an application message of non-zero "
        "length" )
    {
        const auto dup = true;
        const auto qos = packet::QoS::EXACTLY_ONCE;
        const auto retain = false;
        const auto topic = "just/another/different/test/topic"s;
        const auto pktid = std::uint16_t{0x01FE};
        const auto msg = std::vector<uint8_t>{'m', 'e', 's', 's', 'a', 'g', 'e'};

        const auto expected_remaining_length = 2 + ( 2 + topic.length( ) ) + msg.size( );

        WHEN( "a caller calls publish::create(...)" )
        {
            const auto publish_ptr = publish::create( dup, qos, retain, topic, pktid, msg );

            THEN( "it should receive a properly constructed publish packet" )
            {
                CHECK( publish_ptr->dup( ) == dup );
                CHECK( publish_ptr->qos( ) == qos );
                CHECK( publish_ptr->retain( ) == retain );
                CHECK( publish_ptr->topic( ) == topic );
                CHECK( publish_ptr->packet_identifier( ) == pktid );
                CHECK( publish_ptr->remaining_length( ) == expected_remaining_length );
                REQUIRE( publish_ptr->application_message( ) == msg );
            }
        }
    }

    GIVEN(
        "publish packet properties dup, retain, qos, topic, packet identifier and an application message of non-zero "
        "length using QoS 0" )
    {
        const auto dup = true;
        const auto qos = packet::QoS::AT_MOST_ONCE;
        const auto retain = false;
        const auto topic = "just/another/different/test/topic"s;
        const auto pktid = std::uint16_t{0x01FE};
        const auto msg = std::vector<uint8_t>{'m', 'e', 's', 's', 'a', 'g', 'e'};

        const auto expected_remaining_length = ( 2 + topic.length( ) ) + msg.size( );

        WHEN( "a caller calls publish::create(...)" )
        {
            const auto publish_ptr = publish::create( dup, qos, retain, topic, pktid, msg );

            THEN( "it should receive a properly constructed publish packet" )
            {
                CHECK( publish_ptr->dup( ) == dup );
                CHECK( publish_ptr->qos( ) == qos );
                CHECK( publish_ptr->retain( ) == retain );
                CHECK( publish_ptr->topic( ) == topic );
                CHECK( publish_ptr->packet_identifier( ) == pktid );
                CHECK( publish_ptr->remaining_length( ) == expected_remaining_length );
                REQUIRE( publish_ptr->application_message( ) == msg );
            }
        }
    }
}
