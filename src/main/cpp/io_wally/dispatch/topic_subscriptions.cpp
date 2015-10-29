#include "io_wally/dispatch/topic_subscriptions.hpp"

#include <cassert>
#include <string>

#include <boost/log/trivial.hpp>

#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/protocol/suback_packet.hpp"

namespace io_wally
{
    namespace dispatch
    {
        using namespace std;
        using lvl = boost::log::trivial::severity_level;

        // --------------------------------------------------------------------------------
        // struct subscription_container
        // --------------------------------------------------------------------------------

        subscription_container::subscription_container( const std::string& topic_filterp,
                                                        const protocol::packet::QoS maximum_qosp,
                                                        const std::string& client_idp )
            : topic_filter{topic_filterp}, maximum_qos{maximum_qosp}, client_id{client_idp}
        {
        }

        bool subscription_container::matches( const std::string& topic ) const
        {
            auto t_idx = size_t{0};
            auto tf_idx = size_t{0};
            while ( ( t_idx < topic.length( ) ) && ( tf_idx < topic_filter.length( ) ) )
            {
                auto cur_tf_char = topic_filter[tf_idx];
                auto cur_t_char = topic[t_idx];
                switch ( cur_tf_char )
                {
                    case '+':  // single-level wildcard
                    {
                        // + matches empty topic level
                        if ( cur_t_char != '/' )
                        {
                            while ( ( topic[t_idx] != '/' ) && ( t_idx < topic.length( ) ) )
                                ++t_idx;
                        }
                        ++tf_idx;
                    }
                    break;
                    case '#':  // multi-level wildcard
                    {
                        return true;
                    }
                    break;
                    case '/':  // topic separator
                    default:   // regular character
                    {
                        if ( cur_t_char != cur_tf_char )
                        {
                            return false;
                        }
                        ++t_idx;
                        ++tf_idx;
                    }
                }
            }
            // If both topic_filter and topic are exhausted when we get here, the match has succeeded.
            if ( ( t_idx == topic.length( ) ) && ( tf_idx == topic_filter.length( ) ) )
            {
                return true;
            }
            // Special case: wildcard character "#" represents the PARENT and any number of child levels. Since it also
            // matches the PARENT level, "sport/tennis/player1/#" matches "sport/tennis/player1" (SIC!).
            return ( ( tf_idx < topic_filter.length( ) - 1 ) &&
                     ( topic_filter.compare( topic_filter.length( ) - 2, 2, "/#" ) == 0 ) );
        }

        // --------------------------------------------------------------------------------
        // class topic_subscriptions
        // --------------------------------------------------------------------------------

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
