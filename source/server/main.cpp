#include "io_wally/app/application.hpp"

auto main( int argc, char** argv ) -> int
{
    auto app = io_wally::app::application{};

    return app.run( argc, argv );
}
