#include "io_wally/app/logging.hpp"

#include <sstream>

#include <boost/program_options.hpp>

#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include "io_wally/logging_support.hpp"
#include "io_wally/context.hpp"

namespace io_wally
{
    namespace options = boost::program_options;

    namespace app
    {
        /// \see: http://www.boost.org/doc/libs/1_55_0/libs/log/example/doc/tutorial_file.cpp
        void init_logging( const options::variables_map& config )
        {
            namespace sinks = boost::log::sinks;

            boost::log::add_common_attributes( );

            boost::log::register_simple_formatter_factory<boost::log::trivial::severity_level, char>( "Severity" );

            std::ostringstream log_file_name;
            log_file_name << config[context::LOG_FILE].as<const string>( ) << "_%N.log";
            boost::log::add_file_log(
                keywords::file_name = log_file_name.str( ),
                keywords::rotation_size = 10 * 1024 * 1024,
                keywords::time_based_rotation = sinks::file::rotation_at_time_point( 0, 0, 0 ),
                keywords::format =
                    "[%TimeStamp%] [%ProcessID%] [%ThreadID%] [%LineID%] [%Channel%] | %Severity% | %Message%",
                keywords::auto_flush = true );

            if ( config[context::LOG_CONSOLE].as<bool>( ) )
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
