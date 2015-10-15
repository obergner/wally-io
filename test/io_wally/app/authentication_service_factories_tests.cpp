#include "catch.hpp"

#include "io_wally/app/authentication_service_factories.hpp"

SCENARIO( "authentication_service_factories", "[authentication]" )
{
    GIVEN( "the authentication_service_factories instance" )
    {
        const io_wally::app::authentication_service_factories& under_test =
            io_wally::app::authentication_service_factories::instance( );

        WHEN( "a client asks for ACCEPT_ALL authentication_service_factory" )
        {
            const io_wally::spi::authentication_service_factory& accept_all =
                under_test[io_wally::app::authentication_service_factories::ACCEPT_ALL];

            THEN( "it should receive a fully functional accept all authentication_service_factory" )
            {
                const boost::program_options::variables_map config;

                std::unique_ptr<io_wally::spi::authentication_service> accept_all_auth_srvc = accept_all( config );

                const boost::optional<const std::string> usr = std::string( "usr" );
                const boost::optional<const std::string> pwd = std::string( "pwd" );

                REQUIRE( accept_all_auth_srvc->authenticate( "", usr, pwd ) );
            }
        }

        WHEN( "a client asks for a non-existent authentication_service_factory" )
        {
            const io_wally::spi::authentication_service_factory& accept_all =
                under_test[io_wally::app::authentication_service_factories::ACCEPT_ALL];

            THEN( "it should see std::out_of_range being thrown" )
            {
                REQUIRE_THROWS_AS( under_test["does-not-exist"], std::out_of_range );
            }
        }
    }
}
