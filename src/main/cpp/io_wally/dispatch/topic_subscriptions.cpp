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

        // struct subscriptions_container

        topic_subscriptions::subscription_container::subscription_container( const std::string& topic_filterp,
                                                                             const protocol::packet::QoS maximum_qosp,
                                                                             const std::string& client_idp )
            : topic_filter{topic_filterp}, maximum_qos{maximum_qosp}, client_id{client_idp}
        {
        }

        bool topic_subscriptions::subscription_container::matches( const std::string& topic ) const
        {
            auto t_idx = size_t{0};
            auto tf_idx = size_t{0};
            while ( ( t_idx < topic.length( ) ) && ( tf_idx < topic_filter.length( ) ) )
            {
                auto cur_tf_char = topic_filter[t_idx];
                switch ( cur_tf_char )
                {
                    case '/':
                    {
                        if ( topic[t_idx] != '/' )
                        {
                            return false;
                        }
                        ++t_idx;
                        ++tf_idx;
                    }
                    break;
                    case '+':
                    {
                        auto next_slash = topic.find( '/', t_idx );
                        if ( next_slash == std::string::npos )
                        {
                            // No more topic levels left in "topic". We have eaten it.
                            ++t_idx;
                        }
                        else
                        {
                            t_idx = next_slash;
                        }
                        ++tf_idx;
                    }
                    break;
                    case '#':
                    {
                        return true;
                    }
                    break;
                    default:  // regular character
                    {
                        if ( topic[t_idx] != cur_tf_char )
                        {
                            return false;
                        }
                        ++t_idx;
                        ++tf_idx;
                    }
                }
            }
            return ( ( t_idx == topic.length( ) ) && ( tf_idx == topic_filter.length( ) ) );
        }
    }  // namespace dispatch
}  // namespace io_wally
