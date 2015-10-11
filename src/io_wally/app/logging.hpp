#pragma once

#include <boost/program_options.hpp>

namespace io_wally
{
    namespace app
    {
        /// \brief Initialise logging subsystem.
        ///
        /// From \c log_file, initialise a Boost.Log file sink. If \c enable_console_log is \c true, also enable
        /// console logging.
        ///
        /// WallyIO uses Boost.Log v2 for all its logging.
        ///
        /// \param config    Configuration to use when setting up logging
        void init_logging( const boost::program_options::variables_map& config );
    }
}
