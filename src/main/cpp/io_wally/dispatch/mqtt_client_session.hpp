#pragma once

#include <string>
#include <memory>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/protocol/common.hpp"
#include "io_wally/mqtt_connection_handle.hpp"

namespace io_wally
{
    namespace dispatch
    {
        // Forward decl to resolve circular dependency
        class mqtt_client_session_manager;

        class mqtt_client_session final
        {
           public:  // static
            using ptr = std::shared_ptr<mqtt_client_session>;

            static mqtt_client_session::ptr create( mqtt_client_session_manager& session_manager,
                                                    const std::string& client_id,
                                                    std::weak_ptr<mqtt_connection_handle> connection );

           public:
            mqtt_client_session( mqtt_client_session_manager& session_manager,
                                 const std::string& client_id,
                                 std::weak_ptr<mqtt_connection_handle> connection );

            /// \brief ID of client connected to this \c mqtt_client_session.
            ///
            /// \return ID of client connected to this \c mqtt_client_session
            const std::string& client_id( ) const;

            /// \brief Send an \c mqtt_packet to connected client.
            ///
            /// \param packet MQTT packet to send
            void send( protocol::mqtt_packet::ptr packet );

            /// \brief Destroy this \c mqtt_client_session.
            void destroy( );

           private:
            mqtt_client_session_manager& session_manager_;
            const std::string client_id_;
            std::weak_ptr<mqtt_connection_handle> connection_;
            /// Our severity-enabled channel logger
            boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_{
                boost::log::keywords::channel = "client-session:" + client_id_,
                boost::log::keywords::severity = boost::log::trivial::trace};
        };  // class mqtt_client_session
    }       // namespace dispatch
}  // namespace io_wally
