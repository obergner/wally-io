#include "catch.hpp"

#include <memory>
#include <optional>
#include <string>

#include <cxxopts.hpp>

#include "framework/factories.hpp"

#include "io_wally/impl/accept_all_authentication_service_factory.hpp"
#include "io_wally/spi/authentication_service_factory.hpp"

using namespace io_wally;

SCENARIO( "accept_all_authentication_service_factory", "[authentication]" )
{
    io_wally::impl::accept_all_authentication_service_factory under_test;

    GIVEN( "an accept_all_authentication_service instance" )
    {
        const cxxopts::ParseResult config = framework::create_parse_result( );
        std::unique_ptr<io_wally::spi::authentication_service> auth_srvc = under_test( config );

        WHEN( "a client passes in any credentials" )
        {
            const std::optional<const std::string> usr = std::string( "anonymous" );
            const std::optional<const std::string> pwd = std::string( "password" );
            const bool authenticated = auth_srvc->authenticate( "", usr, pwd );

            THEN( "it should see authentication succeed" )
            {
                REQUIRE( authenticated );
            }
        }
    }
}
