#include "catch.hpp"

#include <cstdint>
#include <vector>

#include "io_wally/protocol/suback_packet.hpp"

using namespace io_wally::protocol;

SCENARIO( "suback", "[packets]" )
{
    GIVEN( "a suback packet with packet identifier and 4 return codes" )
    {
        auto packet_id = uint16_t{9};
        auto return_codes =
            std::vector<suback_return_code>{suback_return_code::MAXIMUM_QOS0, suback_return_code::MAXIMUM_QOS1,
                                            suback_return_code::MAXIMUM_QOS2, suback_return_code::FAILURE};
        auto under_test = suback{packet_id, return_codes};

        WHEN( "a caller asks for the packet identifier" )
        {
            auto packet_id_ret = under_test.packet_identifier( );

            THEN( "it should see correct packet identifier returned" )
            {
                REQUIRE( packet_id_ret == packet_id );
            }
        }

        WHEN( "a caller asks for suback packet's return codes" )
        {
            auto return_codes_ret = under_test.return_codes( );

            THEN( "it should see correct return codes returned" )
            {
                REQUIRE( return_codes_ret == return_codes );
            }
        }
    }
}
