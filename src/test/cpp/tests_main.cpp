#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <boost/log/core.hpp>

int main( int argc, char const** argv )
{
    // Disable all logging during unit tests
    boost::log::core::get( )->set_logging_enabled( false );

    return Catch::Session( ).run( argc, argv );
}
