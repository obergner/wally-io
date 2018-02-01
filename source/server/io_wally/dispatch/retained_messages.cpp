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
        // Private static
        // ------------------------------------------------------------------------------------------------------------

        bool retained_messages::subscribe_matches_topic( std::shared_ptr<protocol::subscribe>& subscribe,
                                                         const std::string& topic )
        {
            return std::any_of( subscribe->subscriptions( ).begin( ), subscribe->subscriptions( ).end( ),
                                [topic]( const protocol::subscription& subscription ) {
                                    return topic_filter_matches_topic( subscription.topic_filter( ), topic );
                                } );
        }

        // ------------------------------------------------------------------------------------------------------------
        // Public
        // ------------------------------------------------------------------------------------------------------------

        void retained_messages::retain( std::shared_ptr<protocol::publish> incoming_publish )
        {
            assert( incoming_publish->retain( ) );

            if ( incoming_publish->application_message( ).size( ) > 0 )
            {
                messages_[incoming_publish->topic( )] = incoming_publish;
            }
            else
            {
                messages_.erase( incoming_publish->topic( ) );
            }
        }

        std::vector<std::shared_ptr<protocol::publish>> retained_messages::messages_for(
            std::shared_ptr<protocol::subscribe> incoming_subscribe ) const
        {
            auto matches = std::vector<std::shared_ptr<protocol::publish>>{};
            for ( const auto& topic_publish : messages_ )
            {
                if ( subscribe_matches_topic( incoming_subscribe, topic_publish.first ) )
                {
                    matches.push_back( topic_publish.second );
                }
            }

            return matches;
        }

        std::size_t retained_messages::size( ) const
        {
            return messages_.size( );
        }
    }  // namespace dispatch
}  // namespace io_wally
