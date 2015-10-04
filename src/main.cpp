#include "io_wally/app/application.hpp"

static io_wally::app::application& application = io_wally::app::application::instance( );

int main( int argc, char** argv )
{
    return application.run( argc, argv );
}
