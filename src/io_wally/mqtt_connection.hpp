#pragma once

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/asio.hpp>

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
    using namespace std;

    class mqtt_connection_manager;

    struct mqtt_connection_id
    {
       public:
        mqtt_connection_id( const string username, const boost::asio::ip::tcp::endpoint& client )
            : username_( username ), client_( client )
        {
            return;
        }

        const string& username( )
        {
            return username_;
        }

        const boost::asio::ip::tcp::endpoint& client( )
        {
            return client_;
        }

       private:
        const string username_;
        const boost::asio::ip::tcp::endpoint& client_;
    };

    typedef vector<uint8_t>::iterator buf_iter;

    ///  \brief An MQTT client connection.
    ///
    /// Represents a persistent connection between a client and an \c mqtt_server.
    class mqtt_connection : public boost::enable_shared_from_this<mqtt_connection>
    {
       public:
        /// A pointer to an \c mqtt_connection.
        typedef boost::shared_ptr<mqtt_connection> pointer;

        /// Factory method for \c mqtt_connections.
        static pointer create( boost::asio::ip::tcp::socket socket,
                               mqtt_connection_manager& session_manager,
                               const context& context );

        /// Naturally, mqtt_connections cannot be copied.
        mqtt_connection( const mqtt_connection& ) = delete;
        /// Naturally, mqtt_connections cannot be copied.
        mqtt_connection& operator=( const mqtt_connection& ) = delete;

        /// TODO: I would like to make this destructor private, just like the constructor. Yet boost::shared_ptr
        /// requires a public destructor.
        ~mqtt_connection( );

        struct mqtt_connection_id* id( ) const;

        /// \brief Start this session, initiating reading incoming data.
        void start( );

        /// \brief Stop this session, closing its \c tcp::socket.
        void stop( );

        /// \brief Return a string representation to be used in log output.
        ///
        /// \return A string representation to be used in log output
        const string to_string( ) const;

       private:
        /// Hide constructor since we MUST be created by static factory method 'create' above
        explicit mqtt_connection( boost::asio::ip::tcp::socket socket,
                                  mqtt_connection_manager& session_manager,
                                  const context& context );

        void read_header( );

        void on_header_data_read( const boost::system::error_code& ec, const size_t bytes_transferred );

        void read_body( const decoder::header_decoder::result<buf_iter>& header_parse_result,
                        const size_t bytes_transferred );

        void on_body_data_read( const decoder::header_decoder::result<buf_iter>& header_parse_result,
                                const boost::system::error_code& ec,
                                const size_t bytes_transferred );

        void dispatch_decoded_packet( const protocol::mqtt_packet& packet );

        void write_packet( const protocol::mqtt_packet& packet );

        void write_packet_and_close_session( const protocol::mqtt_packet& packet, const string& message );

       private:
        /// Our ID, only assigned once we have been authenticated
        struct mqtt_connection_id* id_;

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
            keywords::channel = "session",
            keywords::severity = lvl::trace};

        /// The client socket this session is connected to
        boost::asio::ip::tcp::socket socket_;

        /// Our session manager, responsible for managing our lifecycle
        mqtt_connection_manager& session_manager_;

        /// Our context reference, used for configuring ourselves etc
        const context& context_;

        /// Buffer incoming data
        vector<uint8_t> read_buffer_;

        /// Buffer outgoing data
        vector<uint8_t> write_buffer_;
    };

    inline ostream& operator<<( ostream& output, mqtt_connection const& mqtt_connection )
    {
        output << mqtt_connection.to_string( );

        return output;
    }
}
