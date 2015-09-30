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
        /// \brief Utility class for reading program options from command line and configuration file.
        class options_parser
        {
           public:
            const pair<const options::variables_map, const options::options_description> parse( int argc,
                                                                                                char** argv ) const;

        };  // class options_parser
    }       // namespace app
}
