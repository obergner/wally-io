#include <sstream>

#include "io_wally/logging.hpp"

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

// See: http://www.boost.org/doc/libs/1_55_0/libs/log/example/doc/tutorial_file.cpp

namespace io_wally
{
    namespace logging
    {
        const char* to_string( lvl::severity_level lvl )
        {
            switch ( lvl )
            {
                case lvl::trace:
                    return "trace";
                case lvl::debug:
                    return "debug";
                case lvl::info:
                    return "info ";
                case lvl::warn:
                    return "warn ";
                case lvl::error:
                    return "error";
                case lvl::fatal:
                    return "fatal";
                default:
                    return nullptr;
            }
        }

        void init_logging( const std::string& log_file, bool enable_console_log )
        {
            ::logging::add_common_attributes( );

            std::ostringstream log_file_name;
            log_file_name << log_file << "_%N.log";
            ::logging::add_file_log(
                keywords::file_name = log_file_name.str( ),
                keywords::rotation_size = 10 * 1024 * 1024,
                keywords::time_based_rotation = sinks::file::rotation_at_time_point( 0, 0, 0 ),
                keywords::format = "[%TimeStamp%] [%ProcessID%] [%ThreadID%] [%LineID%] | %Severity% | %Message%" );

            if ( enable_console_log )
            {
                ::logging::add_console_log(
                    std::clog,
                    keywords::format = "[%TimeStamp%] [%ProcessID%] [%ThreadID%] [%LineID%] | %Severity% | %Message%" );
            }

            //::logging::core::get( )->set_filter( ::logging::trivial::severity >= ::logging::trivial::info );
        }
    }
}
