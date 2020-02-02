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

namespace io_wally::dispatch
{
    using namespace std;
    // --------------------------------------------------------------------------------
    // struct subscription_container
    // --------------------------------------------------------------------------------

    subscription_container::subscription_container( std::string topic_filter,
                                                    const protocol::packet::QoS maximum_qosp,
                                                    std::string client_id )
        : topic_filter{std::move( topic_filter )}, maximum_qos{maximum_qosp}, client_id{std::move( client_id )}
    {
    }

    auto subscription_container::matches( const std::string& topic ) const -> bool
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

    auto topic_subscriptions::subscribe( const std::string& client_id,
                                         const std::shared_ptr<const protocol::subscribe>& subscribe )
        -> std::shared_ptr<const protocol::suback>
    {
        for ( const auto& subscr : subscribe->subscriptions( ) )
        {
            subscriptions_.emplace( subscr.topic_filter( ), subscr.maximum_qos( ), client_id );
        }
        const auto suback = subscribe->succeed( );
        logger_->debug( "SUBSRCRIBED: [cltid:{}|subscr:{}] -> {}", client_id, *subscribe, *suback );

        return suback;
    }

    auto topic_subscriptions::unsubscribe( const std::string& client_id,
                                           const std::shared_ptr<const protocol::unsubscribe>& unsubscribe )
        -> std::shared_ptr<const protocol::unsuback>
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
        const auto unsuback = unsubscribe->ack( );
        logger_->debug( "UNSUBSRCRIBED: [cltid:{}|unsubscr:{}] -> {}", client_id, *unsubscribe, *unsuback );

        return unsuback;
    }

    auto topic_subscriptions::resolve_subscribers( const std::shared_ptr<const protocol::publish>& publish ) const
        -> const std::vector<resolved_subscriber_t>
    {
        const auto& topic = publish->topic( );

        auto resolved_subscribers = std::vector<resolved_subscriber_t>{};
        for ( const auto& subscr : subscriptions_ )
        {
            if ( !subscr.matches( topic ) )
                continue;
            const auto& seen_subscr = std::find_if(
                resolved_subscribers.begin( ), resolved_subscribers.end( ),
                [&subscr]( const resolved_subscriber_t& res_subscr ) { return res_subscr.first == subscr.client_id; } );
            if ( seen_subscr != resolved_subscribers.end( ) )
            {
                const auto seen_qos = seen_subscr->second;
                if ( subscr.maximum_qos > seen_qos )
                    seen_subscr->second = subscr.maximum_qos;
            }
            else
            {
                resolved_subscribers.emplace_back( resolved_subscriber_t{subscr.client_id, subscr.maximum_qos} );
            }
        }

        return resolved_subscribers;
    }
}  // namespace io_wally::dispatch
