#include "catch.hpp"

#include "io_wally/impl/accept_all_authentication_service_factory.hpp"

using namespace io_wally;

SCENARIO( "accept_all_authentication_service_factory", "[authentication]" )
{
    io_wally::impl::accept_all_authentication_service_factory under_test;

    GIVEN( "an accept_all_authentication_service instance" )
    {
        const boost::program_options::variables_map config;
        unique_ptr<io_wally::spi::authentication_service> auth_srvc = under_test( config );

        WHEN( "a client passes in any credentials" )
        {
            const boost::optional<const string> usr = string( "anonymous" );
            const boost::optional<const string> pwd = string( "password" );
            const bool authenticated = auth_srvc->authenticate( "", usr, pwd );

            THEN( "it should see authentication succeed" )
            {
                REQUIRE( authenticated );
            }
        }
    }
}
