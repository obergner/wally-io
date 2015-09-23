#pragma once

#include <cstdint>

#include <boost/program_options.hpp>

#include <io_wally/spi/authentication_service_factory.hpp>

using namespace std;

namespace options = boost::program_options;

namespace io_wally
{

    class application
    {
       public:
        int run( int argc, char** argv );

       public:
        static const int EC_OK = 0;
        static const int EC_MALFORMED_CMDLINE = 1;
        static const int EC_RUNTIME_ERROR = 2;

       private:
        static const uint32_t DEFAULT_CONNECT_TIMEOUT_MS = 1000;
        static const size_t DEFAULT_INITIAL_READ_BUFFER_SIZE = 256;
        static const size_t DEFAULT_INITIAL_WRITE_BUFFER_SIZE = 256;

       private:
        const pair<const options::variables_map, const options::options_description> parse_configuration(
            int argc,
            char** argv ) const;

        void configure_logging( const options::variables_map& config ) const;

        unique_ptr<io_wally::spi::authentication_service_factory> get_authentication_service_factory(
            const options::variables_map& config ) const;
    };
}
