#include "catch.hpp"

#include "io_wally/protocol/connack_packet.hpp"

using namespace io_wally::protocol;

SCENARIO( "connack_header", "[packets]" )
{

    GIVEN( "a connack_header with session_present set to false" )
    {
        const bool session_present = false;
        connect_return_code return_code = connect_return_code::NOT_AUTHORIZED;
        const connack_header under_test( session_present, return_code );

        WHEN( "a caller asks for the session_present flag" )
        {
            const bool sp = under_test.is_session_present( );

            THEN( "it should see false" )
            {
                REQUIRE( sp == session_present );
            }
        }
    }

    GIVEN( "a connack_header with session_present set to true" )
    {
        const bool session_present = true;
        connect_return_code return_code = connect_return_code::NOT_AUTHORIZED;
        const connack_header under_test( session_present, return_code );

        WHEN( "a caller asks for the session_present flag" )
        {
            const bool sp = under_test.is_session_present( );

            THEN( "it should see true" )
            {
                REQUIRE( sp == session_present );
            }
        }
    }

    GIVEN( "a connack_header with return_code set to NOT_AUTHORIZED" )
    {
        const bool session_present = true;
        connect_return_code return_code = connect_return_code::NOT_AUTHORIZED;
        const connack_header under_test( session_present, return_code );

        WHEN( "a caller asks for the return_code" )
        {
            connect_return_code const rc = under_test.return_code( );

            THEN( "it should see NOT_AUTHORIZED" )
            {
                REQUIRE( rc == return_code );
            }
        }
    }

    GIVEN( "a connack_header with return_code set to UNACCEPTABLE_PROTOCOL_VERSION" )
    {
        const bool session_present = true;
        connect_return_code return_code = connect_return_code::UNACCEPTABLE_PROTOCOL_VERSION;
        const connack_header under_test( session_present, return_code );

        WHEN( "a caller asks for the return_code" )
        {
            connect_return_code const rc = under_test.return_code( );

            THEN( "it should see UNACCEPTABLE_PROTOCOL_VERSION" )
            {
                REQUIRE( rc == return_code );
            }
        }
    }

    GIVEN( "a connack_header with return_code set to IDENTIFIER_REJECTED" )
    {
        const bool session_present = true;
        connect_return_code return_code = connect_return_code::IDENTIFIER_REJECTED;
        const connack_header under_test( session_present, return_code );

        WHEN( "a caller asks for the return_code" )
        {
            connect_return_code const rc = under_test.return_code( );

            THEN( "it should see IDENTIFIER_REJECTED" )
            {
                REQUIRE( rc == return_code );
            }
        }
    }

    GIVEN( "a connack_header with return_code set to SERVER_UNAVAILABLE" )
    {
        const bool session_present = true;
        connect_return_code return_code = connect_return_code::SERVER_UNAVAILABLE;
        const connack_header under_test( session_present, return_code );

        WHEN( "a caller asks for the return_code" )
        {
            connect_return_code const rc = under_test.return_code( );

            THEN( "it should see SERVER_UNAVAILABLE" )
            {
                REQUIRE( rc == return_code );
            }
        }
    }

    GIVEN( "a connack_header with return_code set to BAD_USERNAME_OR_PASSWORD" )
    {
        const bool session_present = true;
        connect_return_code return_code = connect_return_code::BAD_USERNAME_OR_PASSWORD;
        const connack_header under_test( session_present, return_code );

        WHEN( "a caller asks for the return_code" )
        {
            connect_return_code const rc = under_test.return_code( );

            THEN( "it should see BAD_USERNAME_OR_PASSWORD" )
            {
                REQUIRE( rc == return_code );
            }
        }
    }

    GIVEN( "a connack_header with return_code set to CONNECTION_ACCEPTED" )
    {
        const bool session_present = true;
        connect_return_code return_code = connect_return_code::CONNECTION_ACCEPTED;
        const connack_header under_test( session_present, return_code );

        WHEN( "a caller asks for the return_code" )
        {
            connect_return_code const rc = under_test.return_code( );

            THEN( "it should see CONNECTION_ACCEPTED" )
            {
                REQUIRE( rc == return_code );
            }
        }
    }
}
