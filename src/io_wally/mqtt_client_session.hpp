#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include <boost/log/trivial.hpp>

#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"

namespace io_wally
{
    class mqtt_client_session
    {
       public:  // static
        using ptr = std::shared_ptr<mqtt_client_session>;

        mqtt_client_session::ptr create( const uint16_t client_id, std::shared_ptr<mqtt_packet_sender> packet_sender )
        {
            auto session = mqtt_client_session::ptr(
                new mqtt_client_session( client_id, packet_sender ) );  // TODO: This is sooo old-school

            return session;
        }

       public:
        uint16_t client_id( ) const
        {
            return client_id_;
        }

       private:
        explicit mqtt_client_session( const uint16_t client_id, std::shared_ptr<mqtt_packet_sender> packet_sender )
            : client_id_{client_id}, packet_sender_{packet_sender}
        {
            return;
        }

       private:
        const uint16_t client_id_;
        std::shared_ptr<mqtt_packet_sender> packet_sender_;
    };  // class mqtt_client_session
}  // namespace io_wally
