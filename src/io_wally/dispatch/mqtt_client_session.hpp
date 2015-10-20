#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include <boost/log/trivial.hpp>

#include "io_wally/protocol/common.hpp"

namespace io_wally
{
    namespace dispatch
    {
        class mqtt_client_session
        {
           public:  // static
            using ptr = std::shared_ptr<mqtt_client_session>;

            mqtt_client_session::ptr create( const uint16_t client_id )
            {
                return std::make_shared<mqtt_client_session>( client_id );
            }

           public:
            uint16_t client_id( ) const
            {
                return client_id_;
            }

           private:
            mqtt_client_session( const uint16_t client_id ) : client_id_{client_id}
            {
                return;
            }

           private:
            const uint16_t client_id_;
        };  // class mqtt_client_session
    }       // namespace dispatch
}  // namespace io_wally
