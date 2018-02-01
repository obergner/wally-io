#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"

namespace io_wally
{
    namespace dispatch
    {
        class retained_messages final
        {
           public:  // static
            /// Represents a previously retained PUBLISH matching a given SUBSCRIBE: a pair of PUBLISH packet and
            /// desired QoS stating that (a) the PUBLISH packet contained in that pair matches one of the topics in the
            /// given SUBSCRIBE and (b) that it should be published using the QoS contained in that pair.
            ///
            /// In general, a given SUBSCRIBE packet will contain more than one \c subscription, i.e. pair of \c topic
            /// and desired \c QoS. In this case, MQTT 3.1.1 mandates that the PUBLISH packet be delivered to that
            /// client only once, using the maximum QoS of all matching subscriptions.
            ///
            using resolved_publish_t = std::pair<const std::shared_ptr<protocol::publish>, protocol::packet::QoS>;

           public:
            retained_messages( ) = default;

            retained_messages( const retained_messages& ) = delete;

            retained_messages( retained_messages&& ) = delete;

            retained_messages& operator=( const retained_messages& ) = delete;

            retained_messages& operator=( retained_messages&& ) = delete;

            void retain( std::shared_ptr<protocol::publish> incoming_publish );

            std::vector<resolved_publish_t> messages_for(
                const std::shared_ptr<protocol::subscribe> incoming_subscribe ) const;

            std::size_t size( ) const;

           private:
            std::unordered_map<std::string, std::shared_ptr<protocol::publish>> messages_{};
        };  // class retained_messages
    }       // namespace dispatch
}  // namespace io_wally
