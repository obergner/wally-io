#include <sstream>

#include "io_wally/context.hpp"
#include "io_wally/app/logging.hpp"

namespace sinks = boost::log::sinks;

namespace io_wally
{
    namespace app
    {
        /// \see: http://www.boost.org/doc/libs/1_55_0/libs/log/example/doc/tutorial_file.cpp
        void init_logging( const options::variables_map& config )
        {
            boost::log::add_common_attributes( );

            boost::log::register_simple_formatter_factory<boost::log::trivial::severity_level, char>( "Severity" );

            std::ostringstream log_file_name;
            log_file_name << config[io_wally::context::LOG_FILE].as<const string>( ) << "_%N.log";
            boost::log::add_file_log(
                keywords::file_name = log_file_name.str( ),
                keywords::rotation_size = 10 * 1024 * 1024,
                keywords::time_based_rotation = sinks::file::rotation_at_time_point( 0, 0, 0 ),
                keywords::format =
                    "[%TimeStamp%] [%ProcessID%] [%ThreadID%] [%LineID%] [%Channel%] | %Severity% | %Message%",
                keywords::auto_flush = true );

            if ( config[io_wally::context::LOG_CONSOLE].as<bool>( ) )
            {
                boost::log::add_console_log(
                    std::clog,
                    keywords::format =
                        "[%TimeStamp%] [%ProcessID%] [%ThreadID%] [%LineID%] [%Channel%] | %Severity% | %Message%",
                    keywords::auto_flush = true );
            }

            boost::log::core::get( )->set_filter( boost::log::trivial::severity >= boost::log::trivial::debug );
        }
    }
}
