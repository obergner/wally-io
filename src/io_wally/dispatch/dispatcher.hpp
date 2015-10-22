#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include <boost/asio.hpp>

#include "boost/asio_queue.hpp"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/mqtt_connection.hpp"
#include "io_wally/protocol/connect_packet.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/dispatch/common.hpp"
#include "io_wally/concurrency/io_service_pool.hpp"

namespace io_wally
{
    namespace dispatch
    {
        class dispatcher
        {
           public:  // static
            using ptr = std::shared_ptr<dispatcher>;

           public:
            dispatcher( const context& context ) : context_{context}
            {
                return;
            }

            void dispatch( std::shared_ptr<mqtt_connection> rx_connection, std::shared_ptr<protocol::connect> connect );

            void dispatch( const std::string& client_id,
                           std::shared_ptr<mqtt_connection> rx_connection,
                           std::shared_ptr<protocol::subscribe> subscribe );

           private:
            const context& context_;
            mqtt_connection::packetq_t packetq_{1000000};  // Let's start small ...
            concurrency::io_service_pool dispatcher_pool_{"dispatcher", 1};
            boost::asio::io_service& io_service_{dispatcher_pool_.io_service( )};
            boost::asio::queue_sender<mqtt_connection::packetq_t> packet_sender_{io_service_, &packetq_};
            boost::asio::queue_listener<mqtt_connection::packetq_t> packet_receiver_{io_service_, &packetq_};
        };  // class dispatcher
    }       // namespace dispatch
}  // namespace io_wally
