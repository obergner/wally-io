#pragma once

#include <string>
#include <memory>

#include <boost/system/error_code.hpp>

#include <boost/asio.hpp>

#include "boost/asio_queue.hpp"

#include <boost/log/trivial.hpp>

#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/concurrency/io_service_pool.hpp"
#include "io_wally/dispatch/common.hpp"
#include "io_wally/dispatch/mqtt_client_session_manager.hpp"
#include "io_wally/dispatch/topic_subscriptions.hpp"

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
            using ptr = std::shared_ptr<dispatcher>;

            /// \brief Factory method: create new \c dispatcher instance a return a \c shared_ptr to it.
            ///
            /// \param dispatchq Queue to receive dispatched MQTT packets from
            /// \return \c shared_ptr to new \c dispatcher instance
            static dispatcher::ptr create( mqtt_packet_sender::packetq_t& dispatchq );

           public:
            /// \brief Create new \c dispatcher instance.
            ///
            /// WARNING: Except for maybe in unit test scenarios, client code should not use this constructor directly.
            /// Instead, it should use \c dispatcher::create.
            ///
            /// \param dispatchq Queue to receive dispatched MQTT packets from
            dispatcher( mqtt_packet_sender::packetq_t& dispatchq );

            /// \brief Run this \c dispatcher instance, starting its internal \c concurrency::io_service_pool to pull
            /// newly received MQTT packets from its dispatcher queue for further processing.
            void run( );

            /// \brief Stop this \c dispatcher instance, stopping its internal \c concurrency::io_service_pool.
            ///
            /// \param message Optional message to log when stopping
            void stop( const std::string& message = "" );

           private:
            void do_receive_packet( );

            void handle_packet_received( const boost::system::error_code& ec,
                                         mqtt_packet_sender::packet_container_t::ptr packet_container );

           private:
            mqtt_packet_sender::packetq_t& dispatchq_;
            concurrency::io_service_pool dispatcher_pool_{"dispatcher", 1};
            boost::asio::io_service& io_service_{dispatcher_pool_.io_service( )};
            boost::asio::io_service::strand strand_{io_service_};
            boost::asio::queue_listener<mqtt_packet_sender::packetq_t> packet_receiver_{io_service_, &dispatchq_};
            mqtt_client_session_manager session_manager_{};
            topic_subscriptions topic_subscriptions_{};
            /// Our severity-enabled channel logger
            boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_{
                boost::log::keywords::channel = "dispatcher",
                boost::log::keywords::severity = boost::log::trivial::trace};
        };  // class dispatcher
    }       // namespace dispatch
}  // namespace io_wally
