#include "io_wally/dispatch/topic_subscriptions.hpp"

#include <cassert>

#include <boost/log/trivial.hpp>

#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"

namespace io_wally
{
    namespace dispatch
    {
        using namespace std;
        using lvl = boost::log::trivial::severity_level;

        std::shared_ptr<const protocol::suback> topic_subscriptions::subscribe(
            mqtt_connection::packet_container_t::ptr subscribe )
        {
            assert( subscribe->packet_type( ) == protocol::packet::Type::SUBSCRIBE );

            auto subscribe_packet = subscribe->packetAs<const protocol::subscribe>( );
            BOOST_LOG_SEV( logger_, lvl::debug ) << "SUBSRCIBE:  [cltid:" << subscribe->client_id( )
                                                 << "|subscr:" << *subscribe_packet << "]";

            for ( auto& subscr : subscribe_packet->subscriptions( ) )
            {
                subscriptions_.emplace( subscr.topic_filter( ), subscr.maximum_qos( ), subscribe->client_id( ) );
            }
            auto suback = subscribe_packet->succeed( );

            BOOST_LOG_SEV( logger_, lvl::debug ) << "SUBSRCIBED: [cltid:" << subscribe->client_id( )
                                                 << "|subscr:" << *subscribe_packet << "] -> " << *suback;

            return suback;
        }
    }  // namespace dispatch
}  // namespace io_wally
