#include <sstream>

#include "io_wally/logging.hpp"

namespace sinks = boost::log::sinks;

namespace io_wally
{
    namespace logging
    {

        /// \see: http://www.boost.org/doc/libs/1_55_0/libs/log/example/doc/tutorial_file.cpp
        void init_logging( const std::string& log_file, bool enable_console_log )
        {
            boost::log::add_common_attributes( );

            std::ostringstream log_file_name;
            log_file_name << log_file << "_%N.log";
            boost::log::add_file_log(
                keywords::file_name = log_file_name.str( ),
                keywords::rotation_size = 10 * 1024 * 1024,
                keywords::time_based_rotation = sinks::file::rotation_at_time_point( 0, 0, 0 ),
                keywords::format = "[%TimeStamp%] [%ProcessID%] [%ThreadID%] [%LineID%] | %Severity% | %Message%" );

            if ( enable_console_log )
            {
                boost::log::add_console_log(
                    std::clog,
                    keywords::format = "[%TimeStamp%] [%ProcessID%] [%ThreadID%] [%LineID%] | %Severity% | %Message%" );
            }

            boost::log::core::get( )->set_filter( boost::log::trivial::severity >= boost::log::trivial::debug );
        }
    }
}
