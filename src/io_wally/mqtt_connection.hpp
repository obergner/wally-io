#pragma once

#include <vector>
#include <memory>

#include <boost/asio.hpp>

#include <boost/optional.hpp>

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/context.hpp"
#include "io_wally/logging_support.hpp"

#include "io_wally/protocol/common.hpp"

#include "io_wally/codec/decoder.hpp"
#include "io_wally/codec/mqtt_packet_decoder.hpp"
#include "io_wally/codec/mqtt_packet_encoder.hpp"

#include "io_wally/spi/authentication_service_factory.hpp"

namespace io_wally
{
    class mqtt_connection_manager;

    using buf_iter = std::vector<uint8_t>::iterator;

    ///  \brief An MQTT client connection.
    ///
    /// Represents a persistent connection between a client and an \c mqtt_server.
    class mqtt_connection : public std::enable_shared_from_this<mqtt_connection>
    {
        friend class mqtt_connection_manager;

       public:
        /// A \c shared_ptr to an \c mqtt_connection.
        using ptr = std::shared_ptr<mqtt_connection>;

        /// Factory method for \c mqtt_connections.
        static mqtt_connection::ptr create( boost::asio::ip::tcp::socket socket,
                                            mqtt_connection_manager& session_manager,
                                            const context& context );

        /// Naturally, mqtt_connections cannot be copied.
        mqtt_connection( const mqtt_connection& ) = delete;
        /// Naturally, mqtt_connections cannot be copied.
        mqtt_connection& operator=( const mqtt_connection& ) = delete;

        ~mqtt_connection( ){};

        /// \brief Start this session, initiating reading incoming data.
        void start( );

        /// \brief Stop this session, closing its \c tcp::socket.
        void stop( const std::string& message = "",
                   const boost::log::trivial::severity_level log_level = boost::log::trivial::info );

        /// \brief Return a string representation to be used in log output.
        ///
        /// \return A string representation to be used in log output
        const std::string to_string( ) const;

        inline friend std::ostream& operator<<( std::ostream& output, mqtt_connection const& mqtt_connection )
        {
            output << mqtt_connection.to_string( );

            return output;
        }

       private:
        /// Hide constructor since we MUST be created by static factory method 'create' above
        explicit mqtt_connection( boost::asio::ip::tcp::socket socket,
                                  mqtt_connection_manager& session_manager,
                                  const context& context );

        void do_stop( );

        void read_header( );

        void on_header_data_read( const boost::system::error_code& ec, const size_t bytes_transferred );

        void read_body( const decoder::header_decoder::result<buf_iter>& header_parse_result,
                        const size_t bytes_transferred );

        void on_body_data_read( const decoder::header_decoder::result<buf_iter>& header_parse_result,
                                const boost::system::error_code& ec,
                                const size_t bytes_transferred );

        void dispatch_decoded_packet( const protocol::mqtt_packet& packet );

        void write_packet( const protocol::mqtt_packet& packet );

        void write_packet_and_close_session( const protocol::mqtt_packet& packet, const std::string& message );

        void close_on_keep_alive_timeout( );

       private:
        /// Has this session been authenticated, i.e. received a successful CONNECT request?
        bool authenticated = false;
        /// Somehow we need to parse those headers
        decoder::header_decoder header_decoder_{};
        /// And while we are at it, why not parse the rest of those packets, too?
        const decoder::mqtt_packet_decoder<buf_iter> packet_decoder_{};
        /// Encode outgoing packets
        const encoder::mqtt_packet_encoder<buf_iter> packet_encoder_{};
        /// Our severity-enabled channel logger
        boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_{
            boost::log::keywords::channel = "session",
            boost::log::keywords::severity = boost::log::trivial::trace};
        /// The client socket this session is connected to
        boost::asio::ip::tcp::socket socket_;
        /// Strand used to serialize access to socket and timer
        boost::asio::io_service::strand strand_;
        /// Our session manager, responsible for managing our lifecycle
        mqtt_connection_manager& session_manager_;
        /// Our context reference, used for configuring ourselves etc
        const context& context_;
        /// Buffer incoming data
        std::vector<uint8_t> read_buffer_;
        /// Buffer outgoing data
        std::vector<uint8_t> write_buffer_;
        /// Timer, will fire if connection timeout expires without receiving a CONNECT request
        boost::asio::deadline_timer close_on_connection_timeout_;
        /// Keep alive duration (seconds)
        boost::optional<boost::posix_time::time_duration> keep_alive_ = boost::none;
        /// Timer, will fire if keep alive timeout expires without receiving a message
        boost::asio::deadline_timer close_on_keep_alive_timeout_;
    };  // class mqtt_connection
}
