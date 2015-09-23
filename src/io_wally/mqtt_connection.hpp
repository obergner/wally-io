#pragma once

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "io_wally/logging.hpp"
#include "io_wally/codec/decoder.hpp"
#include "io_wally/codec/mqtt_packet_decoder.hpp"
#include "io_wally/codec/mqtt_packet_encoder.hpp"
#include "io_wally/spi/authentication_service_factory.hpp"

using boost::asio::ip::tcp;

using namespace std;
using namespace io_wally::protocol;
using namespace io_wally::decoder;
using namespace io_wally::encoder;
using namespace io_wally::spi;

namespace io_wally
{
    class mqtt_connection_manager;

    struct mqtt_connection_id
    {
       public:
        mqtt_connection_id( const string username, const tcp::endpoint& client )
            : username_( username ), client_( client )
        {
            return;
        }

        const string& username( )
        {
            return username_;
        }

        const tcp::endpoint& client( )
        {
            return client_;
        }

       private:
        const string username_;
        const tcp::endpoint& client_;
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
        static pointer create( tcp::socket socket,
                               mqtt_connection_manager& session_manager,
                               authentication_service& authentication_service );

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
        explicit mqtt_connection( tcp::socket socket,
                                  mqtt_connection_manager& session_manager,
                                  authentication_service& authentication_service );

        void read_header( );

        void on_header_data_read( const boost::system::error_code& ec, const size_t bytes_transferred );

        void read_body( const header_decoder::result<buf_iter>& header_parse_result, const size_t bytes_transferred );

        void on_body_data_read( const header_decoder::result<buf_iter>& header_parse_result,
                                const boost::system::error_code& ec,
                                const size_t bytes_transferred );

        void dispatch_decoded_packet( const mqtt_packet& packet );

        void write_packet( const mqtt_packet& packet );

        void write_packet_and_close_session( const mqtt_packet& packet, const string& message );

       private:
        /// Our ID, only assigned once we have been authenticated
        struct mqtt_connection_id* id_;

        /// Has this session been authenticated, i.e. received a successfully CONNECT request?
        bool authenticated = false;

        /// Our session manager, responsible for managing our lifecycle
        mqtt_connection_manager& session_manager_;

        /// Initial read buffer capacity
        const size_t initial_buffer_capacity = 256;

        /// Buffer incoming data
        vector<uint8_t> read_buffer_ = vector<uint8_t>( initial_buffer_capacity );

        /// Buffer outgoing data
        vector<uint8_t> write_buffer_ = vector<uint8_t>( initial_buffer_capacity );

        /// The client socket this session is connected to
        tcp::socket socket_;

        /// Somehow we need to parse those headers
        header_decoder header_decoder_ = header_decoder{};

        /// And while we are at it, why not parse the rest of those packets, too?
        mqtt_packet_decoder<buf_iter> packet_decoder_ = mqtt_packet_decoder<buf_iter>{};

        /// Encode outgoing packets
        mqtt_packet_encoder<buf_iter> packet_encoder_ = mqtt_packet_encoder<buf_iter>{};

        /// Handle connect requests
        authentication_service& authentication_service_;

        /// Our severity-enabled channel logger
        boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_ =
            boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level>{
                keywords::channel = "session",
                keywords::severity = lvl::trace};
    };

    inline ostream& operator<<( ostream& output, mqtt_connection const& mqtt_connection )
    {
        output << mqtt_connection.to_string( );

        return output;
    }
}
