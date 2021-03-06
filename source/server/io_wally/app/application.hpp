#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>

#include "io_wally/app/options_factory.hpp"
#include "io_wally/mqtt_connection.hpp"
#include "io_wally/mqtt_server.hpp"

namespace io_wally::app
{
    /// \brief Manage \c mqtt_server lifecycle
    ///
    /// Responsibilities:
    ///
    ///  * Parse command line
    ///  * Load additional settings from config file (if specified)
    ///  * Instantiate and configure \c mqtt_server instance
    ///  * Start \c mqtt_server instance
    ///  * Hold on to \c mqtt_server instance, keeping it alive
    ///  * Forward shutdown requests to \c mqtt_server instance
    ///
    class application final
    {
       public:  // static
        /// Exit code to be returned if everything went fine.
        static constexpr const int EC_OK = 0;

        /// Exit code to be returned if user supplied illegal command line parameters and/or illegal config file
        /// settings.
        static constexpr const int EC_MALFORMED_CMDLINE = 1;

        /// Exit code to be returned if encountering generic runtime error.
        static constexpr const int EC_RUNTIME_ERROR = 2;

       public:
        /// \brief Create new \c application instance.
        ///
        application( ) = default;

        /// \brief Run this application.
        ///
        auto run( int argc, char** argv ) -> int;

        /// \brief Wait until \c mqtt_server instance created by this \c application has started listening on its
        ///        server socket.
        ///
        /// Block calling thread until \c mqtt_server instance created by this \c application has started listening
        /// for incoming connection requests. This method has been introduced to facilitate integration tests yet
        /// may one day prove useful in application code.
        void wait_until_started( );

        /// \brief Shutdown this application, releasing all resources.
        ///
        void stop( const std::string& message = "Shutdown request by application" );

       private:
        /// Signal that this \c application instance has completed its \c run() method.
        std::mutex startup_mutex_{};
        std::condition_variable startup_completed_{};
        const options_factory options_factory_{};
        /// std::shared_ptr "owning" THE reference to out mqtt_server instance.
        ///
        /// NOTE:
        ///
        /// By rights, this should be a unique_ptr. HOWEVER: in mqtt_server we use shared_from_this(), and the
        /// contract around this is that it will return a shared_ptr(this) IFF there is already a shared_ptr
        /// pointing to \c this.
        mqtt_server::ptr server_{};
    };  // class application
}  // namespace io_wally::app
