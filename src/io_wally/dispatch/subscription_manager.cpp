#include "io_wally/dispatch/subscription_manager.hpp"

#include <cassert>
#include <string>
#include <memory>
#include <unordered_set>

#include <boost/log/trivial.hpp>

#include "io_wally/mqtt_connection.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/subscription.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/protocol/suback_packet.hpp"

namespace io_wally
{
    namespace dispatch
    {
        using namespace std;
        using lvl = boost::log::trivial::severity_level;

        std::shared_ptr<const protocol::suback> subscription_manager::subscribe(
            mqtt_connection::packet_container_t::ptr subscribe )
        {
            assert( subscribe->packet_type( ) == protocol::packet::Type::SUBSCRIBE );

            auto subscribe_packet = subscribe->packetAs<const protocol::subscribe>( );
            BOOST_LOG_SEV( logger_, lvl::debug ) << "SUBSRCIBE:  [client_id:" << subscribe->client_id( )
                                                 << "|subscr:" << *subscribe_packet << "]";

            auto return_codes = vector<protocol::suback_return_code>{};
            for ( auto& subscr : subscribe_packet->subscriptions( ) )
            {
                subscriptions_.emplace( subscr.topic_filter( ), subscr.maximum_qos( ), subscribe->client_id( ) );

                protocol::suback_return_code rc;
                switch ( subscr.maximum_qos( ) )
                {
                    case protocol::packet::QoS::AT_MOST_ONCE:
                        rc = protocol::suback_return_code::MAXIMUM_QOS0;
                        break;
                    case protocol::packet::QoS::AT_LEAST_ONCE:
                        rc = protocol::suback_return_code::MAXIMUM_QOS1;
                        break;
                    case protocol::packet::QoS::EXACTLY_ONCE:
                        rc = protocol::suback_return_code::MAXIMUM_QOS2;
                        break;
                    case protocol::packet::QoS::RESERVED:
                        rc = protocol::suback_return_code::FAILURE;
                        break;
                    default:
                        rc = protocol::suback_return_code::FAILURE;
                        break;
                }
                return_codes.push_back( rc );
            }
            auto suback = make_shared<const protocol::suback>( subscribe_packet->packet_identifier( ), return_codes );

            BOOST_LOG_SEV( logger_, lvl::debug ) << "SUBSRCIBED: [client_id:" << subscribe->client_id( )
                                                 << "|subscr:" << *subscribe_packet << "] -> " << *suback;

            return suback;
        }
    }  // namespace dispatch
}  // namespace io_wally
