#pragma once

#include <iostream>
#include <fstream>
#include <cstdint>

#include <boost/program_options.hpp>

using namespace std;
namespace options = boost::program_options;

namespace io_wally
{
    namespace app
    {
        namespace defaults
        {
            static const uint32_t DEFAULT_CONNECT_TIMEOUT_MS = 1000;

            static const size_t DEFAULT_INITIAL_READ_BUFFER_SIZE = 256;

            static const size_t DEFAULT_INITIAL_WRITE_BUFFER_SIZE = 256;

            static const string DEFAULT_CONFIG_FILE = "/etc/mqttd.conf";

            static const string DEFAULT_LOG_FILE = "/var/log/mqttd.log";

            static const string DEFAULT_SERVER_ADDRESS = "0.0.0.0";

            static const int DEFAULT_SERVER_PORT = 1883;

            static const string DEFAULT_AUTHENTICATION_SERVICE_FACTORY = "accept_all";
        }

        const pair<const options::variables_map, const options::options_description> parse_configuration( int argc,
                                                                                                          char** argv );
    }  // namespace app
}
