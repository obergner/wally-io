#pragma once

#include <string>
#include <memory>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/puback_packet.hpp"
#include "io_wally/protocol/pubrec_packet.hpp"
#include "io_wally/protocol/pubcomp_packet.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/dispatch/tx_in_flight_publications.hpp"
#include "io_wally/dispatch/rx_in_flight_publications.hpp"

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
            /// \param maximum_qos Maximum quality of service level to use when publishing
            void publish( std::shared_ptr<protocol::publish> incoming_publish,
                          const protocol::packet::QoS maximum_qos );

            /// \brief Called when client sent a PUBLISH.
            ///
            /// \param incoming_publish PUBLISH packet sent by client
            void client_sent_publish( std::shared_ptr<protocol::publish> incoming_publish );

            /// \brief Called when this client acknowledged a received QoS 1 PUBLISH, i.e. sent a PUBACK
            ///
            /// \param puback PUBACK sent by connected client
            void client_acked_publish( std::shared_ptr<protocol::puback> puback );

            /// \brief Called when this client acknowledged a received QoS 2 PUBLISH, i.e. sent a PUBREC
            ///
            /// \param pubrec PUBREC sent by connected client
            void client_received_publish( std::shared_ptr<protocol::pubrec> pubrec );

            /// \brief Called when this client released a sent QoS 2 PUBLISH, i.e. sent a PUBREL.
            ///
            /// \param pubrel PUBREL sent by connected client
            void client_released_publish( std::shared_ptr<protocol::pubrel> pubrel );

            /// \brief Called when this client completed a received QoS 2 PUBLISH, i.e. sent a PUBCOMP
            ///
            /// \param pubcomp PUBCOMP sent by connected client
            void client_completed_publish( std::shared_ptr<protocol::pubcomp> pubcomp );

            /// \brief Destroy this \c mqtt_client_session.
            void destroy( );

           private:
            mqtt_client_session_manager& session_manager_;
            const std::string client_id_;
            std::weak_ptr<mqtt_packet_sender> connection_;
            tx_in_flight_publications tx_in_flight_publications_;
            rx_in_flight_publications rx_in_flight_publications_;
            /// Our severity-enabled channel logger
            boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_{
                boost::log::keywords::channel = "client-session:" + client_id_,
                boost::log::keywords::severity = boost::log::trivial::trace};
        };  // class mqtt_client_session
    }       // namespace dispatch
}  // namespace io_wally
