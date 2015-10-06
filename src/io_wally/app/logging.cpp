#include "io_wally/app/logging.hpp"

#include <string>
#include <sstream>
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
        }

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

            const string log_file_name = config[context::LOG_FILE].as<const string>( ) + "_%N.log";

            const string log_format =
                "[%TimeStamp%] [%ProcessID%] [%ThreadID%] [%LineID%] [%Channel%] | %Severity% | %Message%";

            // FIXME: We will terminate with an uncaught exception if user specifies invalid log level - BAD
            const string log_file_filter = "%Severity% >= " + config[context::LOG_FILE_LEVEL].as<const string>( );
            const string log_console_filter = "%Severity% >= " + config[context::LOG_CONSOLE_LEVEL].as<const string>( );

            boost::log::settings setts;

            setts["Core"]["DisableLogging"] = false;
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
