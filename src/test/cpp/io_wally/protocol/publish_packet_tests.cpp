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

        WHEN( "a caller sets DUP flag" )
        {
            under_test.set_dup( );

            THEN( "dup() should return true" )
            {
                REQUIRE( under_test.dup( ) == true );
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
                REQUIRE( under_test.qos( ) == packet::QoS::EXACTLY_ONCE );
            }
        }
    }
}
