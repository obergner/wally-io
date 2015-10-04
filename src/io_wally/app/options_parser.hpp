#pragma once

#include <utility>

#include <boost/program_options.hpp>

namespace io_wally
{
    using namespace std;

    namespace app
    {
        /// \brief Utility class for reading program options from command line and configuration file.
        class options_parser
        {
           public:
            const pair<const boost::program_options::variables_map, const boost::program_options::options_description>
            parse( const int argc, const char** argv ) const;

        };  // class options_parser
    }       // namespace app
}
