#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include <system_error>

#include <asio.hpp>
#include <asio/steady_timer.hpp>

#include <optional>

#include "io_wally/context.hpp"
#include "io_wally/logging_support.hpp"
#include "io_wally/mqtt_packet_sender.hpp"

#include "io_wally/logging/logging.hpp"

#include "io_wally/protocol/protocol.hpp"

#include "io_wally/codec/decoder.hpp"
#include "io_wally/codec/mqtt_packet_decoder.hpp"
#include "io_wally/codec/mqtt_packet_encoder.hpp"

#include "io_wally/spi/authentication_service_factory.hpp"

#include "io_wally/dispatch/common.hpp"
#include "io_wally/dispatch/dispatcher.hpp"

namespace io_wally
{
    class mqtt_connection_manager;

    ///  \brief An MQTT client connection.
    ///
    /// Represents a persistent connection between a client and an \c mqtt_server.
    class mqtt_connection final : public mqtt_packet_sender, public std::enable_shared_from_this<mqtt_connection>
    {
        friend class mqtt_connection_manager;

       public:  // static
        /// A \c shared_ptr to an \c mqtt_connection.
        using ptr = std::shared_ptr<mqtt_connection>;

        using buf_iter = std::vector<uint8_t>::iterator;

        /// Factory method for \c mqtt_connections.
        static auto create( asio::ip::tcp::socket socket,
                            mqtt_connection_manager& connection_manager,
                            const context& context,
                            dispatch::dispatcher& dispatcher ) -> mqtt_connection::ptr;

       private:  // static
        static auto endpoint_description( const asio::ip::tcp::socket& socket ) -> const std::string;

        static auto connection_description( const asio::ip::tcp::socket& socket, const std::string& client_id = "ANON" )
            -> const std::string;

       public:
        /// Naturally, mqtt_connections cannot be copied.
        mqtt_connection( const mqtt_connection& ) = delete;
        /// Naturally, mqtt_connections cannot be copied.
        auto operator=( const mqtt_connection& ) -> mqtt_connection& = delete;

        ~mqtt_connection( ) override = default;

        /// \brief Start this connection, initiating reading incoming data.
        void start( );

        inline auto client_id( ) const -> const std::optional<const std::string>& override
        {
            return client_id_;
        }

        /// \brief Send an \c mqtt_packet to connected client.
        void send( protocol::mqtt_packet::ptr packet ) override;

        /// \brief Stop this connection, closing its \c tcp::socket.
        void stop( const std::string& message = "",
                   const spdlog::level::level_enum log_level = spdlog::level::level_enum::info ) override;

        operator const std::string&( ) const override
        {
            return description_;
        }

       private:
        /// Hide constructor since we MUST be created by static factory method 'create' above
        mqtt_connection( asio::ip::tcp::socket socket,
                         mqtt_connection_manager& connection_manager,
                         const context& context,
                         dispatch::dispatcher& dispatcher );

        void do_stop( );

        // Receiving MQTT packets

        void read_frame( );

        void on_frame_read( const std::error_code& ec, const size_t bytes_transferred );

        void decode_packet( const size_t bytes_transferred );

        void on_read_failed( const std::error_code& ec, const size_t bytes_transferred );

        // Processing decoded packets

        void process_decoded_packet( const std::shared_ptr<protocol::mqtt_packet>& packet );

        // Dealing with CONNECT packets

        void process_connect_packet( const std::shared_ptr<protocol::connect>& connect );

        void dispatch_connect_packet( const std::shared_ptr<protocol::connect>& connect );

        // Dealing with DISCONNECT packets

        void process_disconnect_packet( const std::shared_ptr<protocol::disconnect>& disconnect );

        void dispatch_disconnect_packet(
            const std::shared_ptr<protocol::disconnect>& disconnect,
            const dispatch::disconnect_reason disconnect_reason = dispatch::disconnect_reason::client_disconnect );

        // Dealing with non-connection managing packets

        void dispatch_packet( const std::shared_ptr<protocol::mqtt_packet>& packet );

        // Sending MQTT packets

        void write_packet( const protocol::mqtt_packet& packet );

        void write_packet_and_close_connection( const protocol::mqtt_packet& packet,
                                                const std::string& message,
                                                const dispatch::disconnect_reason reason );
        // Dealing with connect timeout

        void close_on_connect_timeout( );

        // Dealing with keep alive

        void close_on_keep_alive_timeout( );

        void handle_keep_alive_timeout( const std::error_code& ec );

        // Dealing with protocol violations and network/server failures

        void connection_close_requested( const std::string& message,
                                         const dispatch::disconnect_reason reason,
                                         const std::error_code& ec = std::error_code{},
                                         const spdlog::level::level_enum log_level = spdlog::level::level_enum::err );

       private:
        /// Connected client's client_id. Only assigned once successful authenticated.
        std::optional<const std::string> client_id_ = std::nullopt;
        /// This connection's current string represenation, used in log output
        std::string description_;
        /// Encode outgoing packets
        const encoder::mqtt_packet_encoder<buf_iter> packet_encoder_{};
        /// Strand used to serialize access to socket and timer
        asio::io_service::strand strand_;
        /// The client socket this connection is connected to
        asio::ip::tcp::socket socket_;
        /// Our connection manager, responsible for managing our lifecycle
        mqtt_connection_manager& connection_manager_;
        /// Our context reference, used for configuring ourselves etc
        const context& context_;
        /// dispatcher for publishing messages
        dispatch::dispatcher& dispatcher_;
        /// Buffer incoming data
        std::vector<uint8_t> read_buffer_;
        /// Read entire MQTT frame
        decoder::frame_reader frame_reader_{read_buffer_};
        /// For decoding mqtt packets, you know
        const decoder::mqtt_packet_decoder packet_decoder_{};
        /// Buffer outgoing data
        std::vector<uint8_t> write_buffer_;
        /// Timer, will fire if connection timeout expires without receiving a CONNECT request
        asio::steady_timer close_on_connection_timeout_;
        /// Keep alive duration (seconds)
        std::optional<std::chrono::duration<uint16_t>> keep_alive_ = std::nullopt;
        /// Timer, will fire if keep alive timeout expires without receiving a message
        asio::steady_timer close_on_keep_alive_timeout_;
        /// Our logger
        std::unique_ptr<spdlog::logger> logger_ =
            context_.logger_factory( ).logger( mqtt_connection::endpoint_description( socket_ ) );
    };  // class mqtt_connection
}  // namespace io_wally
