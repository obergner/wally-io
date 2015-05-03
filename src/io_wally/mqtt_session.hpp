#pragma once

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "io_wally/protocol/parser/common.hpp"
#include "io_wally/protocol/parser/mqtt_packet_parser.hpp"

using boost::asio::ip::tcp;

using namespace io_wally::protocol;
using namespace io_wally::protocol::parser;

namespace io_wally
{
    class mqtt_session : public boost::enable_shared_from_this<mqtt_session>
    {
       public:
        typedef boost::shared_ptr<mqtt_session> pointer;

        static pointer create( tcp::socket socket );

        /// Naturally, mqtt_sessions cannot be copied.
        mqtt_session( const mqtt_session& ) = delete;
        mqtt_session& operator=( const mqtt_session& ) = delete;

        /// TODO: I would like to make this destructor private, just like the constructor. Yet boost::shared_ptr
        /// requires a public destructor.
        ~mqtt_session( );

        void start( );

        void stop( );

       private:
        /// Hide constructor since we MUST be created by static factory method 'create' above
        explicit mqtt_session( tcp::socket socket );

        void read_header( );

        void on_header_data_read( const boost::system::error_code& ec, const size_t bytes_transferred );

        void read_body( const header_parser::result<uint8_t*>& header_parse_result, const size_t bytes_transferred );

        void on_body_data_read( const header_parser::result<uint8_t*>& header_parse_result,
                                const boost::system::error_code& ec,
                                const size_t bytes_transferred );

        /// Initial read buffer capacity
        const size_t initial_buffer_capacity = 256;
        /// The client socket this session is connected to
        tcp::socket socket_;
        std::vector<uint8_t> read_buffer_;
        header_parser header_parser_;
        mqtt_packet_parser<uint8_t*> packet_parser_;
    };
}
