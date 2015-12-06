#include "io_wally/dispatch/retained_messages.hpp"

#include <cassert>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "io_wally/protocol/publish_packet.hpp"

namespace io_wally
{
    namespace dispatch
    {
        void retained_messages::retain( std::shared_ptr<protocol::publish> incoming_publish )
        {
            assert( incoming_publish->retain( ) );

            messages_[incoming_publish->topic( )] = incoming_publish;
        }

        std::size_t retained_messages::size( ) const
        {
            return messages_.size( );
        }
    }  // namespace dispatch
}  // namespace io_wally
