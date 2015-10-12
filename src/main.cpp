#include "io_wally/app/application.hpp"

int main( int argc, char** argv )
{
    io_wally::app::application app;

    return app.run( argc, const_cast<const char**>( argv ) );
}
