#pragma once

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "io_wally/protocol/parser/common.hpp"

using boost::asio::ip::tcp;

using namespace io_wally::protocol;
using namespace io_wally::protocol::parser;

namespace io_wally
{
    class mqtt_session : public boost::enable_shared_from_this<mqtt_session>
    {
       public:
        typedef boost::shared_ptr<mqtt_session> pointer;

        static pointer create( boost::asio::io_service& io_service );

        /// TODO: I would like to make this destructor private, just like the constructor. Yet boost::shared_ptr
        /// requires a public destructor.
        ~mqtt_session( );

        mqtt_session( const mqtt_session& ) = delete;
        mqtt_session& operator=( const mqtt_session& ) = delete;

        void start( );

        void stop( );

       private:
        /// Hide constructor since we MUST be created by static factory method 'create' above
        mqtt_session( boost::asio::io_service& io_service );

        void read_header( );

        void on_header_data_read( const boost::system::error_code& ec, const size_t bytes_transferred );

        void read_body( const header_parser::result<uint8_t*>& header_parse_result );

        void on_body_data_read( const header_parser::result<uint8_t*>& header_parse_result,
                                const boost::system::error_code& ec,
                                const size_t bytes_transferred );

        /// Max fixed header length in bytes
        static const size_t MAX_HEADER_LENGTH = 5;
        /// The client socket this session is connected to
        tcp::socket socket_;
        std::vector<uint8_t> read_buffer_;
        header_parser header_parser_;
        mqtt_packet_parser<uint8_t*> packet_parser_;
    };
}
