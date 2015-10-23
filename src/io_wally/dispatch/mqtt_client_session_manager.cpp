#include "io_wally/dispatch/mqtt_client_session_manager.hpp"

#include <string>
#include <memory>
#include <map>

#include <boost/log/trivial.hpp>

#include "io_wally/mqtt_connection.hpp"
#include "io_wally/dispatch/mqtt_client_session.hpp"

namespace io_wally
{
    namespace dispatch
    {
        using namespace std;
        using lvl = boost::log::trivial::severity_level;

        // ------------------------------------------------------------------------------------------------------------
        // Public
        // ------------------------------------------------------------------------------------------------------------

        mqtt_client_session_manager::mqtt_client_session_manager( )
        {
        }

        void mqtt_client_session_manager::client_connected( const std::string& client_id,
                                                            std::weak_ptr<mqtt_connection> connection )
        {
            auto connection_ptr = shared_ptr<mqtt_connection>( connection );
            if ( connection_ptr )
            {
                BOOST_LOG_SEV( logger_, lvl::info ) << "Client connected: [id:" << client_id
                                                    << "|conn:" << *connection_ptr << "]";
                // TODO: Maybe we shouldn't pass a REFERENCE to this, since we might go away (likewise in
                // mqtt_connection)
                auto session = mqtt_client_session::create( *this, client_id, connection );
                sessions_.emplace( client_id, session );
                BOOST_LOG_SEV( logger_, lvl::info ) << "Session for client [id:" << client_id
                                                    << "|conn:" << *connection_ptr << "] created";
            }
            else
            {
                BOOST_LOG_SEV( logger_, lvl::warning )
                    << "Client connected [id:" << client_id
                    << "], yet connection was immediately closed (network/protocol error)";
            }
        }

        void mqtt_client_session_manager::destroy( mqtt_client_session& session )
        {
            sessions_.erase( session.client_id( ) );
            BOOST_LOG_SEV( logger_, lvl::info ) << "Client session [id:" << session.client_id( ) << "] destroyed";
        }

        void mqtt_client_session_manager::destroy_all( )
        {
            sessions_.clear( );
            BOOST_LOG_SEV( logger_, lvl::info ) << "All client sessions destroyed";
        }
    }  // namespace dispatch
}  // namespace io_wally
