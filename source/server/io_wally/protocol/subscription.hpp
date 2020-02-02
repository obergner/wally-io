#pragma once

#include <sstream>
#include <string>

#include "io_wally/error/protocol.hpp"
#include "io_wally/protocol/common.hpp"

namespace io_wally::protocol
{
    /// \brief Represents a list of MQTT topics a client wishes to subscribe to combined with the maximum
    /// \c packet::QoS an MQTT should use when publishing messages received on one of those topics to this client.
    ///
    /// A \c topic_filter is a hierarchical UTF-8 string pattern, using a forward slash ("/" U+002F) to separate
    /// topic levels. In a \c topic_filter, all UTF-8 characters besides the special wildcard characters "#"
    /// (U+0023) and "+" (U+002B) match themselves if a \c topic_filter is compared to a candidate topic string.
    ///
    /// Two adjacent forward slashes ("//") are legal and indicate a zero-length topic level.
    ///
    /// The wildcard character "#" matches any number of topic levels. Therefore, if present, it MUST be the last
    /// character in a \c topic_filter. Any \c topic_filter  MUST contain at most one "#".
    ///
    /// The wildcard character "+" matches exactly one topic level. Any \c topic_filter may contain zero or more "+"
    /// characters.
    ///
    /// \see http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Topic_wildcards
    struct subscription final
    {
       public:
        /// \brief Create a \c subscription instance from the supplied argument.
        ///
        /// \param topic_filter Topic filter representing list of topics a client wants to subscribe to
        /// \param maximum_qos Maximum \c QOS - Quality of Service - level an MQTT server is allowed to use when
        /// delivering a message published to one of the topics matched by \c topic_filter to the client who sent
        /// this \c subscription.
        ///
        /// \throws error::malformed_mqtt_packet If \c topic_filter is illegal
        subscription( std::string topic_filter, packet::QoS maximum_qos )
            : topic_filter_{std::move( topic_filter )}, maximum_qos_{maximum_qos}
        {
            assert_well_formed_topic_filter( topic_filter_ );
        }

        /// \brief Copy constructor.
        ///
        /// \param other \c subscription to copy
        subscription( const subscription& other ) = default;

        /// \brief Move constructor.
        ///
        /// \param other \c subscription to be moved
        subscription( subscription&& other ) = default;

        /// \brief Copy assignment operator.
        ///
        /// \param other \c subscription to assign from.
        // subscription& operator=( const subscription& other ) = default;

        /// \brief Return this \c subscription's topic filter.
        [[nodiscard]] inline auto topic_filter( ) const -> const std::string&
        {
            return topic_filter_;
        }

        /// \brief Return the maximum quality of service level an MQTT may use when sending messages (published on
        /// one of topics matched by this subscription's topic filter) to the client who owns this \c subscription.
        [[nodiscard]] inline auto maximum_qos( ) const -> packet::QoS
        {
            return maximum_qos_;
        }

        /// \brief Return this \c subscription's string representation.
        ///
        /// \return This \c subscription's string representation.
        [[nodiscard]] inline auto to_string( ) const -> const std::string
        {
            std::ostringstream output;
            output << "subscr:[" << topic_filter_ << "|" << maximum_qos_ << "]";

            return output.str( );
        }

        /// \brief Overload stream output operator for \c subscription.
        ///
        /// Overload stream output operator for \c subscription, primarily meant to facilitate logging.
        inline friend auto operator<<( std::ostream& output, subscription const& subscription ) -> std::ostream&
        {
            output << subscription.to_string( );

            return output;
        }

       private:
        static void assert_well_formed_topic_filter( const std::string& topic_filter )
        {
            const char EMPTY = '\0';

            std::string mqtt_ref( "" );
            bool valid = true;

            // topic filter needs to be at least 1 character in length
            if ( topic_filter.size( ) < 1 )
            {
                valid = false;
                mqtt_ref = "4.7.3-1";
            }

            // yet it must not exceed 65535 characters in length
            else if ( topic_filter.size( ) > 65535 )
            {
                valid = false;
                mqtt_ref = "4.7.3-3";
            }

            const char* previous_char = &EMPTY;
            for ( size_t i = 0; ( i < topic_filter.size( ) ) && valid; ++i )
            {
                const char& cur_char = topic_filter[i];

                // NULL byte not allowed in topic filter
                if ( cur_char == '\0' )
                {
                    valid = false;  // [MQTT-4.7.3-2]
                    mqtt_ref = "4.7.3-2";
                }

                // "#" must be the last character in a topic filter
                else if ( *previous_char == '#' )
                {
                    valid = false;
                    mqtt_ref = "4.7.1-2";
                }

                // "+" must either be the first character in a topic filter, or it must be immediately preceded by a
                // "/"
                else if ( ( cur_char == '+' ) && ( *previous_char != EMPTY ) && ( *previous_char != '/' ) )
                {
                    valid = false;  //  [MQTT-4.7.1-1]
                    mqtt_ref = "4.7.1-1";
                }

                // "#" must either be the first character in a topic filter, or it must be immediately preceded by a
                // "/"
                else if ( ( cur_char == '#' ) && ( *previous_char != EMPTY ) && ( *previous_char != '/' ) )
                {
                    valid = false;  //  [MQTT-4.7.1-1]
                    mqtt_ref = "4.7.1-2";
                }

                // A regular character - not one of "#", "+" and "/" - must not follow a "+" or "#"
                else if ( ( cur_char != '/' ) && ( cur_char != '+' ) && ( cur_char != '#' ) &&
                          ( ( *previous_char == '+' ) || ( *previous_char == '#' ) ) )
                {
                    valid = false;  //  [MQTT-4.7.1-3]
                    mqtt_ref = "4.7.1-3";
                }

                previous_char = &cur_char;
            }

            if ( !valid )
                throw error::malformed_mqtt_packet( "[MQTT-" + mqtt_ref +
                                                    "] Not a well-formed topic filter: " + topic_filter );
        }

       private:
        const std::string topic_filter_;
        const packet::QoS maximum_qos_;
    };  // struct subscription

    /// \brief Overloaded equality operator for \subscription instances to facilitate use in standard containers.
    inline auto operator==( const subscription& lhs, const subscription& rhs ) -> bool
    {
        return ( lhs.topic_filter( ) == rhs.topic_filter( ) ) && ( lhs.maximum_qos( ) == rhs.maximum_qos( ) );
    }
}  // namespace io_wally::protocol
