#pragma once

#include <string>
#include <memory>

#include <boost/log/trivial.hpp>

#include "io_wally/protocol/common.hpp"
#include "io_wally/mqtt_connection.hpp"

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
                                                    std::weak_ptr<mqtt_connection> connection );

           public:
            mqtt_client_session( mqtt_client_session_manager& session_manager,
                                 const std::string& client_id,
                                 std::weak_ptr<mqtt_connection> connection );

            const std::string& client_id( ) const;

            void destroy( );

           private:
            mqtt_client_session_manager& session_manager_;
            const std::string client_id_;
            std::weak_ptr<mqtt_connection> connection_;
        };  // class mqtt_client_session
    }       // namespace dispatch
}  // namespace io_wally
