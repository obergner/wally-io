#pragma once

#include <string>
#include <memory>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/protocol/common.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/dispatch/tx_in_flight_publications.hpp"

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
                                                    std::weak_ptr<mqtt_packet_sender> connection );

           public:
            mqtt_client_session( mqtt_client_session_manager& session_manager,
                                 const std::string& client_id,
                                 std::weak_ptr<mqtt_packet_sender> connection );

            /// \brief ID of client connected to this \c mqtt_client_session.
            ///
            /// \return ID of client connected to this \c mqtt_client_session
            const std::string& client_id( ) const;

            /// \brief Send an \c mqtt_packet to connected client.
            ///
            /// \param packet MQTT packet to send
            void send( protocol::mqtt_packet::ptr packet );

            /// \brief Publish \c incoming_publish to connected client.
            ///
            /// \param incoming_publish PUBLISH packet received from (another) client
            void publish( std::shared_ptr<protocol::publish> incoming_publish );

            /// \brief Called when this client acknowledged a received QoS 1 PUBLISH, i.e. sent a PUBACK
            ///
            /// \param puback PUBACK sent by connected client
            void client_acked_publish( std::shared_ptr<protocol::puback> puback );

            /// \brief Destroy this \c mqtt_client_session.
            void destroy( );

           private:
            mqtt_client_session_manager& session_manager_;
            const std::string client_id_;
            std::weak_ptr<mqtt_packet_sender> connection_;
            tx_in_flight_publications tx_in_flight_publications_;
            /// Our severity-enabled channel logger
            boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_{
                boost::log::keywords::channel = "client-session:" + client_id_,
                boost::log::keywords::severity = boost::log::trivial::trace};
        };  // class mqtt_client_session
    }       // namespace dispatch
}  // namespace io_wally
