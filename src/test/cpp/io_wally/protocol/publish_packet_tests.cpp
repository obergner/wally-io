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
                REQUIRE( under_test.header( ).flags( ).dup( ) == true );
            }
        }
    }
}
