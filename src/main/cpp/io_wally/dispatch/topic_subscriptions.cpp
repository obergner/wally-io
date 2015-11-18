#include "io_wally/dispatch/topic_subscriptions.hpp"

#include <cassert>
#include <string>
#include <algorithm>
#include <vector>

#include <boost/log/trivial.hpp>

#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/protocol/suback_packet.hpp"
#include "io_wally/protocol/publish_packet.hpp"

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
                auto tf_char = topic_filter[tf_idx];
                auto t_char = topic[t_idx];
                switch ( tf_char )
                {
                    case '+':  // single-level wildcard
                    {
                        // + matches empty topic level
                        if ( t_char != '/' )
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
                        if ( t_char != tf_char )
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
            const std::string& client_id,
            std::shared_ptr<const protocol::subscribe> subscribe )
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "SUBSRCIBE:  [cltid:" << client_id << "|subscr:" << *subscribe
                                                 << "]";

            for ( auto& subscr : subscribe->subscriptions( ) )
            {
                subscriptions_.emplace( subscr.topic_filter( ), subscr.maximum_qos( ), client_id );
            }
            auto suback = subscribe->succeed( );

            BOOST_LOG_SEV( logger_, lvl::debug ) << "SUBSRCIBED: [cltid:" << client_id << "|subscr:" << *subscribe
                                                 << "] -> " << *suback;

            return suback;
        }

        const std::vector<resolved_subscriber_t> topic_subscriptions::resolve_subscribers(
            std::shared_ptr<const protocol::publish> publish ) const
        {
            auto const& topic = publish->topic( );

            auto resolved_subscribers = vector<resolved_subscriber_t>{};
            for_each( subscriptions_.begin( ), subscriptions_.end( ),
                      [&topic, &resolved_subscribers]( const subscription_container& subscr )
                      {
                          if ( !subscr.matches( topic ) )
                              return;
                          auto const& seen_subscr = find_if( resolved_subscribers.begin( ), resolved_subscribers.end( ),
                                                             [&subscr]( const resolved_subscriber_t& res_subscr )
                                                             {
                                                                 return res_subscr.first == subscr.client_id;
                                                             } );
                          if ( seen_subscr != resolved_subscribers.end( ) )
                          {
                              auto const seen_qos = seen_subscr->second;
                              if ( subscr.maximum_qos > seen_qos )
                                  seen_subscr->second = subscr.maximum_qos;
                          }
                          else
                          {
                              resolved_subscribers.emplace_back(
                                  resolved_subscriber_t{subscr.client_id, subscr.maximum_qos} );
                          }
                      } );

            return resolved_subscribers;
        }
    }  // namespace dispatch
}  // namespace io_wally
