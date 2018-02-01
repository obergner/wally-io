#include "io_wally/dispatch/retained_messages.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/protocol/subscription.hpp"

#include "io_wally/dispatch/common.hpp"

namespace io_wally
{
    namespace dispatch
    {
        // ------------------------------------------------------------------------------------------------------------
        // Public
        // ------------------------------------------------------------------------------------------------------------

        void retained_messages::retain( std::shared_ptr<protocol::publish> incoming_publish )
        {
            assert( incoming_publish->retain( ) );

            // A PUBLISH with retained flag set and application message size 0 REMOVES any retained PUBLISH
            // previously stored under that topic.
            // See: http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718038
            if ( incoming_publish->application_message( ).size( ) > 0 )
            {
                messages_[incoming_publish->topic( )] = incoming_publish;
            }
            else
            {
                messages_.erase( incoming_publish->topic( ) );
            }
        }

        std::vector<retained_messages::resolved_publish_t> retained_messages::messages_for(
            std::shared_ptr<protocol::subscribe> incoming_subscribe ) const
        {
            auto resolved_publishes = std::vector<retained_messages::resolved_publish_t>{};
            for ( const auto& topic_publish : messages_ )
            {
                for ( const auto& subscr : incoming_subscribe->subscriptions( ) )
                {
                    if ( !topic_filter_matches_topic( subscr.topic_filter( ), topic_publish.first ) )
                        continue;

                    const auto& seen_resolved_publish =
                        std::find_if( resolved_publishes.begin( ), resolved_publishes.end( ),
                                      [&topic_publish]( const resolved_publish_t& res_publish ) {
                                          return res_publish.first == topic_publish.second;
                                      } );
                    if ( seen_resolved_publish != resolved_publishes.end( ) )
                    {
                        const auto seen_qos = seen_resolved_publish->second;
                        if ( subscr.maximum_qos( ) > seen_qos )
                            seen_resolved_publish->second = subscr.maximum_qos( );
                    }
                    else
                    {
                        resolved_publishes.emplace_back( topic_publish.second, subscr.maximum_qos( ) );
                    }
                }
            }

            return resolved_publishes;
        }

        std::size_t retained_messages::size( ) const
        {
            return messages_.size( );
        }
    }  // namespace dispatch
}  // namespace io_wally
