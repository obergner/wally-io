#include "io_wally/app/logging.hpp"

#include <string>
#include <sstream>

#include <boost/program_options.hpp>

#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
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
        /// \see:
        /// http://www.boost.org/doc/libs/develop/libs/log/doc/html/log/detailed/utilities.html#log.detailed.utilities.setup
        void init_logging( const options::variables_map& config )
        {
            boost::log::add_common_attributes( );

            boost::log::register_simple_formatter_factory<boost::log::trivial::severity_level, char>( "Severity" );

            ostringstream log_file_name;
            log_file_name << config[context::LOG_FILE].as<const string>( ) << "_%N.log";

            const string log_format =
                "[%TimeStamp%] [%ProcessID%] [%ThreadID%] [%LineID%] [%Channel%] | %Severity% | %Message%";

            // FIXME: Filter expressions currently do not work using settings container below

            // boost::log::core::get( )->set_filter( boost::log::trivial::severity >= boost::log::trivial::debug );
            boost::log::settings setts;

            setts["Core"]["DisableLogging"] = false;
            // setts["Core"]["Filter"] = "%Severity% >= trace"; FIXME

            setts["Sinks"]["File"]["Destination"] = "TextFile";
            setts["Sinks"]["File"]["FileName"] = log_file_name.str( );
            setts["Sinks"]["File"]["Format"] = log_format;
            setts["Sinks"]["File"]["Asynchronous"] = !config[context::LOG_SYNC].as<const bool>( );
            setts["Sinks"]["File"]["AutoFlush"] = true;
            setts["Sinks"]["File"]["RotationSize"] = 10 * 1024 * 1024;  // 10 MiB

            if ( config[context::LOG_CONSOLE].as<bool>( ) )
            {
                setts["Sinks"]["Console"]["Destination"] = "Console";
                setts["Sinks"]["Console"]["Format"] = log_format;
                setts["Sinks"]["Console"]["AutoFlush"] = true;
                // setts["Sinks.Console"]["Filter"] = "%Severity% >= 0"; FIXME
            }

            boost::log::init_from_settings( setts );
        }
    }
}
