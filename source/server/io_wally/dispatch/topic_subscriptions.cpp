#include "io_wally/dispatch/topic_subscriptions.hpp"

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include "io_wally/dispatch/common.hpp"
#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/suback_packet.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"

namespace io_wally
{
    namespace dispatch
    {
        using namespace std;
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
            return io_wally::dispatch::topic_filter_matches_topic( topic_filter, topic );
        }

        // --------------------------------------------------------------------------------
        // class topic_subscriptions
        // --------------------------------------------------------------------------------

        topic_subscriptions::topic_subscriptions( const context& context )
        {
            logger_ = context.logger_factory( ).logger( "topic-subscriptions" );
        }

        std::shared_ptr<const protocol::suback> topic_subscriptions::subscribe(
            const std::string& client_id,
            std::shared_ptr<const protocol::subscribe> subscribe )
        {
            for ( auto& subscr : subscribe->subscriptions( ) )
            {
                subscriptions_.emplace( subscr.topic_filter( ), subscr.maximum_qos( ), client_id );
            }
            auto suback = subscribe->succeed( );
            logger_->debug( "SUBSRCRIBED: [cltid:{}|subscr:{}] -> {}", client_id, *subscribe, *suback );

            return suback;
        }

        std::shared_ptr<const protocol::unsuback> topic_subscriptions::unsubscribe(
            const std::string& client_id,
            std::shared_ptr<const protocol::unsubscribe> unsubscribe )
        {
            for ( auto it = subscriptions_.begin( ); it != subscriptions_.end( ); )
            {
                if ( it->topic_filter_matches_one_of( unsubscribe->topic_filters( ) ) )
                {
                    it = subscriptions_.erase( it );
                }
                else
                {
                    ++it;
                }
            }
            auto unsuback = unsubscribe->ack( );
            logger_->debug( "UNSUBSRCRIBED: [cltid:{}|unsubscr:{}] -> {}", client_id, *unsubscribe, *unsuback );

            return unsuback;
        }

        const std::vector<resolved_subscriber_t> topic_subscriptions::resolve_subscribers(
            std::shared_ptr<const protocol::publish> publish ) const
        {
            auto const& topic = publish->topic( );

            auto resolved_subscribers = vector<resolved_subscriber_t>{};
            for_each( subscriptions_.begin( ), subscriptions_.end( ),
                      [&topic, &resolved_subscribers]( const subscription_container& subscr ) {
                          if ( !subscr.matches( topic ) )
                              return;
                          auto const& seen_subscr = find_if( resolved_subscribers.begin( ), resolved_subscribers.end( ),
                                                             [&subscr]( const resolved_subscriber_t& res_subscr ) {
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
