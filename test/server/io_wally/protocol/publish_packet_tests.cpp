#include "catch.hpp"

#include <cstdint>
#include <vector>

#include "io_wally/protocol/publish_packet.hpp"

using namespace io_wally::protocol;

SCENARIO( "publish", "[packets]" )
{
    GIVEN( "a publish with DUP set to false" )
    {
        auto const flgs = std::uint8_t{3 << 4 | 0x03};
        auto const rem_len = std::uint32_t{5};
        auto const hdr = packet::header{flgs, rem_len};

        auto const topic = "a";

        auto const pktid = std::uint16_t{0xF312};

        auto const msg = std::vector<uint8_t>{0x00, 0x00};

        auto under_test = publish{hdr, topic, pktid, msg};

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
        auto const flgs = std::uint8_t{3 << 4 | 0x09};
        auto const rem_len = std::uint32_t{5};
        auto const hdr = packet::header{flgs, rem_len};

        auto const topic = "a";

        auto const pktid = std::uint16_t{0xF312};

        auto const msg = std::vector<uint8_t>{0x00, 0x00};

        auto under_test = publish{hdr, topic, pktid, msg};

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
        auto const flgs = std::uint8_t{3 << 4 | 0x00};
        auto const rem_len = std::uint32_t{3};
        auto const hdr = packet::header{flgs, rem_len};

        auto const topic = "a";

        auto const pktid = std::uint16_t{0xF312};

        auto const msg = std::vector<uint8_t>{0x01, 0x02};

        auto under_test = publish{hdr, topic, 0, msg};

        WHEN( "a caller calls qos()" )
        {
            THEN( "it should see 'At most once' returned" )
            {
                CHECK( under_test.retain( ) == false );
                CHECK( under_test.dup( ) == false );
                CHECK( under_test.header( ).remaining_length( ) == rem_len );
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
                REQUIRE( under_test.header( ).remaining_length( ) == rem_len + 2 );
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
                REQUIRE( under_test.header( ).remaining_length( ) == rem_len + 2 );
            }
        }
    }

    GIVEN( "a publish with QoS At least once (QoS1)" )
    {
        auto const flgs = std::uint8_t{3 << 4 | 0x0B};
        auto const rem_len = std::uint32_t{5};
        auto const hdr = packet::header{flgs, rem_len};

        auto const topic = "a";

        auto const pktid = std::uint16_t{0xF312};

        auto const msg = std::vector<uint8_t>{0x00, 0x00};

        auto under_test = publish{hdr, topic, pktid, msg};

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
                CHECK( under_test.header( ).remaining_length( ) == rem_len );
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
                REQUIRE( under_test.header( ).remaining_length( ) == rem_len - 2 );
            }
        }
    }

    GIVEN( "a publish with QoS 'Exactly once' (QoS2)" )
    {
        auto const flgs = std::uint8_t{3 << 4 | 0x04};
        auto const rem_len = std::uint32_t{5};
        auto const hdr = packet::header{flgs, rem_len};

        auto const topic = "a";

        auto const pktid = std::uint16_t{0xF312};

        auto const msg = std::vector<uint8_t>{0x00, 0x00};

        auto under_test = publish{hdr, topic, pktid, msg};

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
                CHECK( under_test.header( ).remaining_length( ) == rem_len );
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
                REQUIRE( under_test.header( ).remaining_length( ) == rem_len - 2 );
            }
        }
    }

    GIVEN( "a publish with RETAIN set to false" )
    {
        auto const flgs = std::uint8_t{3 << 4 | 0x02};
        auto const rem_len = std::uint32_t{5};
        auto const hdr = packet::header{flgs, rem_len};

        auto const topic = "a";

        auto const pktid = std::uint16_t{0xF312};

        auto const msg = std::vector<uint8_t>{0x00, 0x00};

        auto under_test = publish{hdr, topic, pktid, msg};

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
        auto const flgs = std::uint8_t{3 << 4 | 0x03};
        auto const rem_len = std::uint32_t{5};
        auto const hdr = packet::header{flgs, rem_len};

        auto const topic = "a";

        auto const pktid = std::uint16_t{0xF312};

        auto const msg = std::vector<uint8_t>{0x00, 0x00};

        auto under_test = publish{hdr, topic, pktid, msg};

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
