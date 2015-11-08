#pragma once

#include <vector>
#include <memory>
#include <chrono>

#include <boost/system/error_code.hpp>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

#include <boost/optional.hpp>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include <boost/asio_queue.hpp>

#include "io_wally/mqtt_packet_sender.hpp"
#include "io_wally/context.hpp"
#include "io_wally/logging_support.hpp"

#include "io_wally/protocol/protocol.hpp"

#include "io_wally/codec/decoder.hpp"
#include "io_wally/codec/mqtt_packet_decoder.hpp"
#include "io_wally/codec/mqtt_packet_encoder.hpp"

#include "io_wally/spi/authentication_service_factory.hpp"

#include "io_wally/dispatch/common.hpp"

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
        static mqtt_connection::ptr create( boost::asio::ip::tcp::socket socket,
                                            mqtt_connection_manager& connection_manager,
                                            const context& context,
                                            packetq_t& dispatchq );

       public:
        /// Naturally, mqtt_connections cannot be copied.
        mqtt_connection( const mqtt_connection& ) = delete;
        /// Naturally, mqtt_connections cannot be copied.
        mqtt_connection& operator=( const mqtt_connection& ) = delete;

        ~mqtt_connection( ){};

        /// \brief Start this connection, initiating reading incoming data.
        void start( );

        virtual inline const boost::optional<const std::string>& client_id( ) const override
        {
            return client_id_;
        }

        /// \brief Send an \c mqtt_packet to connected client.
        virtual void send( protocol::mqtt_packet::ptr packet ) override;

        /// \brief Stop this connection, closing its \c tcp::socket.
        virtual void stop( const std::string& message = "",
                           const boost::log::trivial::severity_level log_level = boost::log::trivial::info ) override;

        /// \brief Return a string representation to be used in log output.
        ///
        /// \return A string representation to be used in log output
        virtual const std::string to_string( ) const override;

        // inline friend std::ostream& operator<<( std::ostream& output, mqtt_connection const& mqtt_connection )
        // {
        //     output << mqtt_connection.to_string( );

        //     return output;
        // }

       private:
        /// Hide constructor since we MUST be created by static factory method 'create' above
        mqtt_connection( boost::asio::ip::tcp::socket socket,
                         mqtt_connection_manager& connection_manager,
                         const context& context,
                         packetq_t& dispatchq );

        void do_stop( );

        // Receiving MQTT packets

        void read_header( );

        void on_header_data_read( const boost::system::error_code& ec, const size_t bytes_transferred );

        void read_body( const decoder::header_decoder::result<buf_iter>& header_parse_result,
                        const size_t bytes_transferred );

        void on_body_data_read( const decoder::header_decoder::result<buf_iter>& header_parse_result,
                                const boost::system::error_code& ec,
                                const size_t bytes_transferred );

        void on_read_failed( const boost::system::error_code& ec, const size_t bytes_transferred );

        // Processing decoded packets

        void process_decoded_packet( std::shared_ptr<protocol::mqtt_packet> packet );

        // Dealing with CONNECT packets

        void process_connect_packet( std::shared_ptr<protocol::connect> connect );

        void dispatch_connect_packet( std::shared_ptr<protocol::connect> connect );

        void handle_dispatch_connect_packet( const boost::system::error_code& ec,
                                             std::shared_ptr<protocol::connect> connect );

        // Dealing with DISCONNECT packets

        void process_disconnect_packet( std::shared_ptr<protocol::disconnect> disconnect );

        void dispatch_disconnect_packet(
            std::shared_ptr<protocol::disconnect> disconnect,
            const dispatch::disconnect_reason disconnect_reason = dispatch::disconnect_reason::client_disconnect );

        void handle_dispatch_disconnect_packet( const boost::system::error_code& ec,
                                                std::shared_ptr<protocol::disconnect> disconnect,
                                                const dispatch::disconnect_reason disconnect_reason );

        // Dealing with SUBSCRIBE packets

        void process_subscribe_packet( std::shared_ptr<protocol::subscribe> subscribe );

        void dispatch_subscribe_packet( std::shared_ptr<protocol::subscribe> subscribe );

        void handle_dispatch_subscribe_packet( const boost::system::error_code& ec,
                                               std::shared_ptr<protocol::subscribe> subscribe );

        // Dealing with PUBLISH packets

        void process_publish_packet( std::shared_ptr<protocol::publish> publish );

        void dispatch_publish_packet( std::shared_ptr<protocol::publish> publish );

        void handle_dispatch_publish_packet( const boost::system::error_code& ec,
                                             std::shared_ptr<protocol::publish> publish );

        // Dealing with PUBACK packets

        void process_puback_packet( std::shared_ptr<protocol::puback> puback );

        void dispatch_puback_packet( std::shared_ptr<protocol::puback> puback );

        void handle_dispatch_puback_packet( const boost::system::error_code& ec,
                                            std::shared_ptr<protocol::puback> puback );

        // Dealing with PUBREC packets

        void process_pubrec_packet( std::shared_ptr<protocol::pubrec> pubrec );

        void dispatch_pubrec_packet( std::shared_ptr<protocol::pubrec> pubrec );

        void handle_dispatch_pubrec_packet( const boost::system::error_code& ec,
                                            std::shared_ptr<protocol::pubrec> pubrec );

        // Dealing with PUBCOMP packets

        void process_pubcomp_packet( std::shared_ptr<protocol::pubcomp> pubcomp );

        void dispatch_pubcomp_packet( std::shared_ptr<protocol::pubcomp> pubcomp );

        void handle_dispatch_pubcomp_packet( const boost::system::error_code& ec,
                                             std::shared_ptr<protocol::pubcomp> pubcomp );

        // Sending MQTT packets

        void write_packet( const protocol::mqtt_packet& packet );

        void write_packet_and_close_connection( const protocol::mqtt_packet& packet,
                                                const std::string& message,
                                                const dispatch::disconnect_reason reason );

        // Dealing with keep alive

        void close_on_keep_alive_timeout( );

        // Dealing with protocol violations and network/server failures

        void connection_close_requested(
            const std::string& message,
            const dispatch::disconnect_reason reason,
            const boost::system::error_code& ec = boost::system::errc::make_error_code( boost::system::errc::success ),
            const boost::log::trivial::severity_level log_level = boost::log::trivial::error );

       private:
        /// Connected client's client_id. Only assigned once successful authenticated.
        boost::optional<const std::string> client_id_ = boost::none;
        /// Somehow we need to parse those headers
        decoder::header_decoder header_decoder_{};
        /// And while we are at it, why not parse the rest of those packets, too?
        const decoder::mqtt_packet_decoder<buf_iter> packet_decoder_{};
        /// Encode outgoing packets
        const encoder::mqtt_packet_encoder<buf_iter> packet_encoder_{};
        /// The client socket this connection is connected to
        boost::asio::ip::tcp::socket socket_;
        /// Strand used to serialize access to socket and timer
        boost::asio::io_service::strand strand_;
        /// Our connection manager, responsible for managing our lifecycle
        mqtt_connection_manager& connection_manager_;
        /// Our context reference, used for configuring ourselves etc
        const context& context_;
        /// Our dispatcher queue, used for forwarding received protocol packets to dispatcher subsystem
        /// TODO: Consider removing this reference since all we need is dispatcher_ below.
        packetq_t& dispatchq_;
        /// Queue sender for dispatching received protocol packets asynchronously (from point of view of connection) to
        /// dispatcher queue.
        boost::asio::queue_sender<packetq_t> dispatcher_{socket_.get_io_service( ), &dispatchq_};
        /// Buffer incoming data
        std::vector<uint8_t> read_buffer_;
        /// Buffer outgoing data
        std::vector<uint8_t> write_buffer_;
        /// Timer, will fire if connection timeout expires without receiving a CONNECT request
        boost::asio::steady_timer close_on_connection_timeout_;
        /// Keep alive duration (seconds)
        boost::optional<std::chrono::duration<uint16_t>> keep_alive_ = boost::none;
        /// Timer, will fire if keep alive timeout expires without receiving a message
        boost::asio::steady_timer close_on_keep_alive_timeout_;
        /// Our severity-enabled channel logger
        boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_{
            boost::log::keywords::channel =
                "connection:" + ( socket_.is_open( )
                                      ? socket_.remote_endpoint( ).address( ).to_string( ) + "/" +
                                            std::to_string( socket_.remote_endpoint( ).port( ) )
                                      : "CLOSED" ),
            boost::log::keywords::severity = boost::log::trivial::trace};
    };  // class mqtt_connection
}
