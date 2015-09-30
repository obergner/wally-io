#pragma once

#include <cstdint>

#include <boost/program_options.hpp>

#include "io_wally/mqtt_server.hpp"
#include "io_wally/app/options_parser.hpp"
#include "io_wally/spi/authentication_service_factory.hpp"

using namespace std;

namespace options = boost::program_options;

namespace io_wally
{
    namespace app
    {
        class application
        {
           public:
            static constexpr const int EC_OK = 0;
            static constexpr const int EC_MALFORMED_CMDLINE = 1;
            static constexpr const int EC_RUNTIME_ERROR = 2;

           public:
            static application& instance( )
            {
                static application instance;

                return instance;
            }

           public:
            int run( int argc, char** argv );

           private:
            application( )
            {
                return;
            }

           private:
            const options_parser options_parser_{};
            unique_ptr<io_wally::mqtt_server> server_{};
        };  // class application
    }       // namespace app
}  // namespace io_wally
