#pragma once

#include <string>
#include <memory>
#include <unordered_set>
#include <functional>

#include <boost/log/trivial.hpp>

#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/subscription.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/protocol/suback_packet.hpp"
#include "io_wally/mqtt_connection.hpp"
#include "io_wally/dispatch/mqtt_client_session.hpp"
#include "io_wally/dispatch/mqtt_client_session_manager.hpp"

namespace io_wally
{
    namespace dispatch
    {
        /// \brief Manager for \c topic \c subscriptions.
        ///
        /// Manages all \c protocol::subscription (topic subscription) instances received from clients:
        ///
        /// - Will be forwarded all \c protocol::subscribe packets received from clients and add its \c
        /// protocol::subscriptions (topic subscriptions) to an internally managed set containing all such
        /// subscriptions.
        /// - Will be forwarded all \c protocol::publish packets received from clients and will respond with the set of
        /// \c mqtt_client_session instances that subscribe to the topic the packet has been published to.
        ///
        class topic_subscriptions final
        {
           public:
            topic_subscriptions( ) = default;

            topic_subscriptions( const topic_subscriptions& ) = delete;

            topic_subscriptions& operator=( topic_subscriptions ) = delete;

            std::shared_ptr<const protocol::suback> subscribe( mqtt_connection::packet_container_t::ptr subscribe );

           private:  // nested types
            struct subscription_container final
            {
               public:
                subscription_container( const std::string& topic_filterp,
                                        const protocol::packet::QoS maximum_qosp,
                                        const std::string& client_idp );

                inline bool operator==( const subscription_container& other ) const
                {
                    return ( topic_filter == other.topic_filter ) && ( maximum_qos == other.maximum_qos ) &&
                           ( client_id == other.client_id );
                }

                /// \brief Test if supplied \c topic matches this \c topic \c filter.
                ///
                /// \param topic Topic to match against this \c topic \c filter
                /// \return \c true if \c topic matches this \c topic \c filter, \c false otherwise
                ///
                /// \pre \c topic is a well-formed topic string
                bool matches( const std::string& topic ) const;

               public:
                const std::string topic_filter;
                const protocol::packet::QoS maximum_qos;
                const std::string client_id;
            };  // struct subscription_container

            /// \brief Hash functor. Needed for \c subscription_container to be usable in \c std::unordered_set.
            struct subscription_container_hash
            {
                std::size_t operator( )( const subscription_container& subscr ) const
                {
                    return std::hash<std::string>( )( subscr.topic_filter ) ^
                           static_cast<std::size_t>( subscr.maximum_qos ) ^
                           std::hash<std::string>( )( subscr.client_id );
                }
            };  // struct subscription_container_hash

           private:
            // Set of subscriptions we manage (cannot be <const subscription_container> since members of std containers
            // in C++ need to be copy assignable or movable, which is incompatible with const)
            std::unordered_set<subscription_container, subscription_container_hash> subscriptions_{1000};
            /// Our severity-enabled channel logger
            boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_{
                boost::log::keywords::channel = "topic-subscriptions",
                boost::log::keywords::severity = boost::log::trivial::trace};
        };  // class topic_subscriptions
    }       // namespace dispatch
}  // namespace io_wally
