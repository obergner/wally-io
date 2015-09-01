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
    class mqtt_session_manager;

    struct mqtt_session_id
    {
       public:
        mqtt_session_id( const string username, const tcp::endpoint& client ) : username_( username ), client_( client )
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

    ///  \brief An MQTT client connection.
    ///
    /// Represents a persistent connection between a client and an \c mqtt_server.
    class mqtt_session : public boost::enable_shared_from_this<mqtt_session>
    {
       public:
        /// A pointer to an \c mqtt_session.
        typedef boost::shared_ptr<mqtt_session> pointer;

        /// Factory method for \c mqtt_sessions.
        static pointer create( tcp::socket socket,
                               mqtt_session_manager& session_manager,
                               authentication_service& authentication_service );

        /// Naturally, mqtt_sessions cannot be copied.
        mqtt_session( const mqtt_session& ) = delete;
        /// Naturally, mqtt_sessions cannot be copied.
        mqtt_session& operator=( const mqtt_session& ) = delete;

        /// TODO: I would like to make this destructor private, just like the constructor. Yet boost::shared_ptr
        /// requires a public destructor.
        ~mqtt_session( );

        struct mqtt_session_id* id( ) const;

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
        explicit mqtt_session( tcp::socket socket,
                               mqtt_session_manager& session_manager,
                               authentication_service& authentication_service );

        void read_header( );

        void on_header_data_read( const boost::system::error_code& ec, const size_t bytes_transferred );

        void read_body( const header_decoder::result<uint8_t*>& header_parse_result, const size_t bytes_transferred );

        void on_body_data_read( const header_decoder::result<uint8_t*>& header_parse_result,
                                const boost::system::error_code& ec,
                                const size_t bytes_transferred );

        void dispatch_decoded_packet( const mqtt_packet& packet );

       private:
        /// Our ID, only assigned once we have been authenticated
        struct mqtt_session_id* id_;

        /// Our session manager, responsible for managing our lifecycle
        mqtt_session_manager& session_manager_;

        /// Initial read buffer capacity
        const size_t initial_buffer_capacity = 256;

        /// Buffer incoming data
        vector<uint8_t> read_buffer_;

        /// The client socket this session is connected to
        tcp::socket socket_;

        /// Somehow we need to parse those headers
        header_decoder header_decoder_;

        /// And while we are at it, why not parse the rest of those packets, too?
        mqtt_packet_decoder<uint8_t*> packet_decoder_;

        /// Encode outgoing packets
        mqtt_packet_encoder<uint8_t*> packet_encoder_;

        /// Handle connect requests
        authentication_service& authentication_service_;

        /// Our severity-enabled channel logger
        boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_;
    };

    inline ostream& operator<<( ostream& output, mqtt_session const& mqtt_session )
    {
        output << mqtt_session.to_string( );

        return output;
    }
}
