#pragma once

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "io_wally/logging.hpp"
#include "io_wally/protocol/codec/parser.hpp"
#include "io_wally/protocol/codec/mqtt_packet_parser.hpp"

using boost::asio::ip::tcp;

using namespace io_wally::protocol;
using namespace io_wally::protocol::parser;

namespace io_wally
{
    class mqtt_session_manager;

    class mqtt_session : public boost::enable_shared_from_this<mqtt_session>
    {
       public:
        typedef boost::shared_ptr<mqtt_session> pointer;

        static pointer create( tcp::socket socket, mqtt_session_manager& session_manager );

        /// Naturally, mqtt_sessions cannot be copied.
        mqtt_session( const mqtt_session& ) = delete;
        mqtt_session& operator=( const mqtt_session& ) = delete;

        /// TODO: I would like to make this destructor private, just like the constructor. Yet boost::shared_ptr
        /// requires a public destructor.
        ~mqtt_session( );

        void start( );

        void stop( );

        /// \brief Return a string representation to be used in log output.
        ///
        /// \return A string representation to be used in log output
        const std::string to_string( ) const;

       private:
        /// Hide constructor since we MUST be created by static factory method 'create' above
        explicit mqtt_session( tcp::socket socket, mqtt_session_manager& session_manager );

        void read_header( );

        void on_header_data_read( const boost::system::error_code& ec, const size_t bytes_transferred );

        void read_body( const header_parser::result<uint8_t*>& header_parse_result, const size_t bytes_transferred );

        void on_body_data_read( const header_parser::result<uint8_t*>& header_parse_result,
                                const boost::system::error_code& ec,
                                const size_t bytes_transferred );

        /// Our session manager, responsible for managing our lifecycle
        mqtt_session_manager& session_manager_;

        /// Initial read buffer capacity
        const size_t initial_buffer_capacity = 256;

        /// Buffer incoming data
        std::vector<uint8_t> read_buffer_;

        /// The client socket this session is connected to
        tcp::socket socket_;

        /// Somehow we need to parse those headers
        header_parser header_parser_;

        /// And while we are at it, why not parse the rest of those packets, too?
        mqtt_packet_parser<uint8_t*> packet_parser_;

        /// Our severity-enabled channel logger
        boost::log::sources::severity_channel_logger<boost::log::trivial::severity_level> logger_;
    };

    inline std::ostream& operator<<( std::ostream& output, mqtt_session const& mqtt_session )
    {
        output << mqtt_session.to_string( );

        return output;
    }
}
