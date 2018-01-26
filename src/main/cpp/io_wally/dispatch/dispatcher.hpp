#pragma once

#include <memory>
#include <string>

#include <system_error>

#include <asio.hpp>

#include <spdlog/spdlog.h>

#include "io_wally/context.hpp"
#include "io_wally/dispatch/common.hpp"
#include "io_wally/dispatch/mqtt_client_session_manager.hpp"
#include "io_wally/logging/logging.hpp"
#include "io_wally/mqtt_packet_sender.hpp"

namespace io_wally
{
    namespace dispatch
    {
        /// \brief Responsible for processing MQTT packet received on dispatcher queue, forwarding them appropriately.
        ///
        /// Depending on its type, received MQTT packets will be forwarded to one of several further components:
        ///
        ///  - CONNECT:     CONNECT packets will be forwarded to \c mqtt_client_session_manager which will create a new
        ///                 \c mqtt_client_session
        ///  - SUBSCRIBE:   XXX
        ///  - PUBLISH:     XXX
        ///
        /// Note that \c dispatcher is an *active* component: it manages its own internal \c
        /// concurrency::io_service_pool, used for pulling MQTT packets received on the network subsystem from a shared
        /// dispatcher queue.
        class dispatcher final : public std::enable_shared_from_this<dispatcher>
        {
           public:  // static
            /// \brief Create new \c dispatcher instance.
            ///
            /// \param context Context containing our configuration
            /// \param io_service Asio io_service instance to be passed on to \c mqtt_client_session_manager
            dispatcher( const context& context, ::asio::io_service& io_service );

           public:
            void handle_packet_received( mqtt_packet_sender::packet_container_t::ptr packet_container );

            /// \brief Stop this \c dispatcher instance, closing all \c mqtt_client_sessions
            ///
            /// \param message Optional message to log when stopping
            /// TODO: Consider using RAII for this, i.e. move to destructor
            void stop( const std::string& message = "" );

           private:
            mqtt_client_session_manager session_manager_;
            std::unique_ptr<spdlog::logger> logger_ = logging::logger_factory::get( ).logger( "dispatcher" );
        };  // class dispatcher
    }       // namespace dispatch
}  // namespace io_wally
