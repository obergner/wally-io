#include "io_wally/dispatch/common.hpp"

#include <cstdint>
#include <string>

namespace io_wally::dispatch
{
    auto topic_filter_matches_topic( const std::string& topic_filter, const std::string& topic ) -> bool
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
}  // namespace io_wally::dispatch
