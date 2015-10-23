#include "io_wally/dispatch/mqtt_client_session.hpp"

#include <string>
#include <memory>

#include <boost/log/trivial.hpp>

#include "io_wally/protocol/common.hpp"
#include "io_wally/mqtt_connection.hpp"
#include "io_wally/dispatch/mqtt_client_session_manager.hpp"

namespace io_wally
{
    namespace dispatch
    {
        // ------------------------------------------------------------------------------------------------------------
        // Public
        // ------------------------------------------------------------------------------------------------------------

        mqtt_client_session::ptr mqtt_client_session::create( mqtt_client_session_manager& session_manager,
                                                              const std::string& client_id,
                                                              std::weak_ptr<mqtt_connection> connection )
        {
            return std::make_shared<mqtt_client_session>( session_manager, client_id, connection );
        }

        mqtt_client_session::~mqtt_client_session( )
        {
            destroy( );
        }

        mqtt_client_session::mqtt_client_session( mqtt_client_session_manager& session_manager,
                                                  const std::string& client_id,
                                                  std::weak_ptr<mqtt_connection> connection )
            : session_manager_{session_manager}, client_id_{client_id}, connection_{connection}
        {
        }

        const std::string& mqtt_client_session::client_id( ) const
        {
            return client_id_;
        }

        void mqtt_client_session::destroy( )
        {
            session_manager_.destroy( *this );
        }
    }  // namespace dispatch
}  // namespace io_wally
