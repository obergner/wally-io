#include "io_wally/app/application.hpp"

int main( int argc, char** argv )
{
    auto app = io_wally::app::application{};

    return app.run( argc, argv );
}
