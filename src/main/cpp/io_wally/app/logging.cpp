#include "io_wally/app/logging.hpp"

#include <string>
#include <algorithm>
#include <iostream>
#include <exception>

#include <boost/program_options.hpp>

#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/exception_handler.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/log/utility/setup/filter_parser.hpp>
#include <boost/log/utility/setup/settings.hpp>
#include <boost/log/utility/setup/from_settings.hpp>

#include "io_wally/logging_support.hpp"
#include "io_wally/context.hpp"
#include "io_wally/defaults.hpp"

namespace io_wally
{
    using namespace std;
    namespace options = boost::program_options;

    namespace app
    {
        namespace
        {
            struct log_exception_handler
            {
                void operator( )( const exception& ex ) const
                {
                    cerr << "ERROR (logging subsystem): " << ex.what( ) << endl;
                }
            };

            // Vector of supported log levels
            const vector<string> SUPPORTED_LOG_LEVELS = {"trace", "debug", "info", "warning", "error", "fatal"};

            bool is_supported_log_level( const string& log_level )
            {
                return find( SUPPORTED_LOG_LEVELS.begin( ), SUPPORTED_LOG_LEVELS.end( ), log_level ) !=
                       SUPPORTED_LOG_LEVELS.end( );
            }

            const string safe_log_level_filter( const string& log_level, const string& default_level )
            {
                if ( !is_supported_log_level( log_level ) )
                {
                    cerr << "ERROR: log level \"" << log_level
                         << "\" is not supported and will be replaced with default log level \"" << default_level
                         << "\"" << endl
                         << "HINT:  supported log levels are: (trace|debug|info|warning|error|fatal)" << endl;

                    return "%Severity% >= " + default_level;
                }

                return "%Severity% >= " + log_level;
            }
        }  // namespace

        /// \see:
        /// http://www.boost.org/doc/libs/develop/libs/log/doc/html/log/detailed/utilities.html#log.detailed.utilities.setup
        void init_logging( const options::variables_map& config )
        {
            // Register exception handler as early as possible to report any errors during subsequent initialization
            boost::log::core::get( )->set_exception_handler(
                boost::log::make_exception_handler<exception>( log_exception_handler( ) ) );

            boost::log::add_common_attributes( );

            // https://groups.google.com/forum/#!topic/boost-list/oa--RefYCYE
            boost::log::register_simple_formatter_factory<boost::log::trivial::severity_level, char>( "Severity" );
            boost::log::register_simple_filter_factory<boost::log::trivial::severity_level, char>( "Severity" );

            auto log_file_name = config[context::LOG_FILE].as<const string>( ) + "_%N.log";

            auto log_format =
                "[%TimeStamp%] [%ProcessID%] [%ThreadID%] [%LineID%] [%Channel%] | %Severity% | %Message%";

            auto log_file_filter = safe_log_level_filter( config[context::LOG_FILE_LEVEL].as<const string>( ),
                                                          defaults::DEFAULT_LOG_FILE_LEVEL );
            auto log_console_filter = safe_log_level_filter( config[context::LOG_CONSOLE_LEVEL].as<const string>( ),
                                                             defaults::DEFAULT_LOG_CONSOLE_LEVEL );

            auto setts = boost::log::settings{};

            setts["Core"]["DisableLogging"] = config[context::LOG_DISABLE].as<const bool>( );
            setts["Core"]["Filter"] = log_file_filter;

            setts["Sinks"]["File"]["Destination"] = "TextFile";
            setts["Sinks"]["File"]["FileName"] = log_file_name;
            setts["Sinks"]["File"]["Format"] = log_format;
            setts["Sinks"]["File"]["Asynchronous"] = !config[context::LOG_SYNC].as<const bool>( );
            setts["Sinks"]["File"]["AutoFlush"] = true;
            setts["Sinks"]["File"]["RotationSize"] = 10 * 1024 * 1024;  // 10 MiB

            if ( config[context::LOG_CONSOLE].as<bool>( ) )
            {
                setts["Sinks"]["Console"]["Destination"] = "Console";
                setts["Sinks"]["Console"]["Format"] = log_format;
                setts["Sinks"]["Console"]["AutoFlush"] = true;
                setts["Sinks"]["Console"]["Filter"] = log_console_filter;
            }

            boost::log::init_from_settings( setts );
        }
    }
}
