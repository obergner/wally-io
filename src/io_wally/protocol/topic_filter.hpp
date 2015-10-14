#pragma once

#include <string>

#include "common.hpp"

namespace io_wally
{
    namespace protocol
    {
        /// \b Represents a list of MQTT topics a client wishes to subscribe to.
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
        struct topic_filter
        {
           public:
            /// \brief Create a \c topic_filter instance from the supplied argument.
            ///
            /// \param topic_filter String representation of \c topic_filter to create
            ///
            /// TODO: check that parameter is a well-formed topic filter. Otherwise, throw an appropriate custom
            /// exception.
            topic_filter( const std::string topic_filter ) : topic_filter_{topic_filter}
            {
                return;
            }

            /// \brief Copy constructor.
            ///
            /// \param other \c topic_filter to copy
            topic_filter( const topic_filter& other ) = default;

            /// \brief Move constructor.
            ///
            /// \param other \c topic_filter to be moved
            topic_filter( topic_filter&& other ) = default;

            /// \brief Copy assignment operator.
            ///
            /// \param other \c topic_filter to assign from.
            topic_filter& operator=( const topic_filter& other ) = default;

            /// \brief Move assignment operator.
            ///
            /// \param other \c topic_filter to move from.
            topic_filter& operator=( topic_filter&& other ) = default;

            /// \brief Test if this \c topic_filter matches \c topic.
            ///
            /// \param topic The topic to match this \c topic_filter against.
            /// \return \c true if \c topic matches this \c topic_filter, \c false otherwise
            bool matches( const std::string& topic ) const
            {
                // TODO: implement
                return true;
            }

            /// \brief Return this \c topic_filter's string representation.
            ///
            /// \return This \c topic_filter's string representation.
            inline const std::string to_string( ) const
            {
                return topic_filter_;
            }

           private:
            const std::string topic_filter_;
        };  // struct topic_filter

        /// \brief Overload stream output operator for \c topic_filter.
        ///
        /// Overload stream output operator for \c topic_filter, primarily meant to facilitate logging.
        inline std::ostream& operator<<( std::ostream& output, topic_filter const& topic_filter )
        {
            output << topic_filter.to_string( );

            return output;
        }
    }  // namespace protocol
}  // namespace io_wally
