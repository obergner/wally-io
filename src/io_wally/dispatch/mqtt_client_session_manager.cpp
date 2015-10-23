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

        mqtt_client_session_manager::~mqtt_client_session_manager( )
        {
            destroy_all( );
        }

        void mqtt_client_session_manager::client_connected( const std::string& client_id,
                                                            std::weak_ptr<mqtt_connection> connection )
        {
            auto connection_ptr = shared_ptr<mqtt_connection>( connection );
            if ( connection_ptr )
            {
                BOOST_LOG_SEV( logger_, lvl::debug ) << "Client connected: [id:" << client_id
                                                     << "|conn:" << *connection_ptr << "]";
                // TODO: Maybe we shouldn't pass a REFERENCE to this, since we might go away (likewise in
                // mqtt_connection)
                auto session = mqtt_client_session::create( *this, client_id, connection );
                sessions_.emplace( client_id, session );
                BOOST_LOG_SEV( logger_, lvl::info ) << "Session for client [id:" << client_id
                                                    << "|conn:" << *connection_ptr
                                                    << "] created [total:" << sessions_.size( ) << "]";
            }
            else
            {
                BOOST_LOG_SEV( logger_, lvl::warning )
                    << "Client connected [id:" << client_id
                    << "], yet connection was immediately closed (network/protocol error)";
            }
        }

        void mqtt_client_session_manager::client_disconnected( const std::string client_id )
        {
            sessions_.erase( client_id );
            BOOST_LOG_SEV( logger_, lvl::debug ) << "Client disconnected: [id:" << client_id << "] - session destroyed";
        }

        void mqtt_client_session_manager::destroy( const std::string client_id )
        {
            auto rem_cnt = sessions_.erase( client_id );
            if ( rem_cnt > 0 )
                BOOST_LOG_SEV( logger_, lvl::info ) << "Client session [id:" << client_id << "] destroyed";
        }

        void mqtt_client_session_manager::destroy_all( )
        {
            auto sess_cnt = sessions_.size( );
            sessions_.clear( );
            BOOST_LOG_SEV( logger_, lvl::info ) << "SHUTDOWN: [" << sess_cnt << "] client session(s) destroyed";
        }
    }  // namespace dispatch
}  // namespace io_wally
