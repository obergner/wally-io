#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "io_wally/protocol/publish_packet.hpp"

namespace io_wally
{
    namespace dispatch
    {
        class retained_messages final
        {
           public:
            retained_messages( ) = default;

            retained_messages( const retained_messages& ) = delete;

            retained_messages( retained_messages&& ) = delete;

            retained_messages& operator=( const retained_messages& ) = delete;

            retained_messages& operator=( retained_messages&& ) = delete;

            void retain( std::shared_ptr<protocol::publish> incoming_publish );

            std::size_t size( ) const;

           private:
            std::unordered_map<std::string, std::shared_ptr<protocol::publish>> messages_{};
        };  // class retained_messages
    }       // namespace dispatch
}  // namespace io_wally
