#pragma once

#include <string>
#include <memory>
#include <unordered_set>
#include <tuple>
#include <vector>
#include <functional>
#include <algorithm>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/subscription.hpp"
#include "io_wally/protocol/subscribe_packet.hpp"
#include "io_wally/protocol/suback_packet.hpp"
#include "io_wally/protocol/unsubscribe_packet.hpp"
#include "io_wally/protocol/unsuback_packet.hpp"
#include "io_wally/protocol/publish_packet.hpp"
#include "io_wally/dispatch/common.hpp"

namespace io_wally
{
    namespace dispatch
    {
        /// \brief Value type combining \c topic \c filter, \c maximum \c QoS and \c client \c id to represent an MQTT
        ///        client's subscription to a set of topics.
        ///
        /// NOTE: This class is public (for now) to facilitate unit tests since it encapsulates logic for matching a \c
        /// topic against a \c topic \c filter. There are surely other, more elegant ways to achieve the same without
        /// breaking encapsulation, but here I chose the easy way out.
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

            /// \brief Test if this \c subscription's topic filter matches one of the \c topic_filters.
            bool topic_filter_matches_one_of( const std::vector<std::string>& topic_filters ) const
            {
                return std::find( topic_filters.begin( ), topic_filters.end( ), topic_filter ) != topic_filters.end( );
            }

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
                       static_cast<std::size_t>( subscr.maximum_qos ) ^ std::hash<std::string>( )( subscr.client_id );
            }
        };  // struct subscription_container_hash

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

            /// \brief Register all subscriptions contained in \c subscribe for client \c client_id.
            ///
            /// \param client_id ID of client that wants to subscribe
            /// \param subscribe SUBSCRIBE packet
            /// \return SUBACK packet
            std::shared_ptr<const protocol::suback> subscribe( const std::string& client_id,
                                                               std::shared_ptr<const protocol::subscribe> subscribe );

            /// \brief Unsubscribe client \c client_id from all topic filters in \c unsubscribe.
            ///
            /// \param client_id ID of client that wants to cancel subscriptions
            /// \param unsubscribe UNSUBSCRIBE packet containing topic filters to cancel
            /// \return UNSUBACK packet
            std::shared_ptr<const protocol::unsuback> unsubscribe(
                const std::string& client_id,
                std::shared_ptr<const protocol::unsubscribe> unsubscribe );

            /// \brief Determine set of clients subscribed to \c topic packet \c publish is published to.
            ///
            /// \param publish PUBLISH packet for which we want to determine all subscribers
            ///
            /// TODO: Investigate if returning a std::vector by value has negative performance impacts, and if so, fix
            /// this.
            const std::vector<resolved_subscriber_t> resolve_subscribers(
                std::shared_ptr<const protocol::publish> publish ) const;

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
