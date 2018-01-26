#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <io_wally/logging/logging.hpp>

int main( int argc, char** argv )
{
    // Disable all logging during unit tests
    io_wally::logging::logger_factory::disable( );

    return Catch::Session( ).run( argc, argv );
}
