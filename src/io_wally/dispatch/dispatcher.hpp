#pragma once

#include <string>
#include <memory>

#include <boost/system/error_code.hpp>

#include <boost/asio.hpp>

#include "boost/asio_queue.hpp"

#include <boost/log/trivial.hpp>

#include "io_wally/mqtt_connection.hpp"
#include "io_wally/dispatch/common.hpp"
#include "io_wally/concurrency/io_service_pool.hpp"

namespace io_wally
{
    namespace dispatch
    {
        class dispatcher : public std::enable_shared_from_this<dispatcher>
        {
           public:  // static
            using ptr = std::shared_ptr<dispatcher>;

            static dispatcher::ptr create( mqtt_connection::packetq_t& dispatchq );

           public:
            dispatcher( mqtt_connection::packetq_t& dispatchq );

            void run( );

            void stop( const std::string& message = "" );

           private:
            void do_receive_packet( );

            void handle_packet_received( const boost::system::error_code& ec,
                                         mqtt_connection::packet_container_t::ptr packet_container );

           private:
            mqtt_connection::packetq_t& dispatchq_;  // Let's start small ...
            concurrency::io_service_pool dispatcher_pool_{"dispatcher", 1};
            boost::asio::io_service& io_service_{dispatcher_pool_.io_service( )};
            boost::asio::io_service::strand strand_{io_service_};
            boost::asio::queue_listener<mqtt_connection::packetq_t> packet_receiver_{io_service_, &dispatchq_};
            /// Our severity-enabled channel logger
            boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_{
                boost::log::keywords::channel = "dispatcher",
                boost::log::keywords::severity = boost::log::trivial::trace};
        };  // class dispatcher
    }       // namespace dispatch
}  // namespace io_wally
