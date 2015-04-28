#include <vector>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "io_wally/protocol/connect_packet.hpp"
#include "io_wally/protocol/parser/common.hpp"

using boost::asio::ip::tcp;

using namespace io_wally::protocol::parser;

namespace io_wally
{
    class mqtt_session : public boost::enable_shared_from_this<mqtt_session>
    {
       public:
        typedef boost::shared_ptr<mqtt_session> pointer;

        static pointer create( boost::asio::io_service& io_service )
        {
            return pointer( new mqtt_session( io_service ) );
        }

        mqtt_session( const mqtt_session& ) = delete;
        mqtt_session& operator=( const mqtt_session& ) = delete;

        void start( )
        {
            do_read( );
        }

        void stop( )
        {
            socket_.close( );
        }

       private:
        /// Hide constructor since we MUST be created by static factory method 'create' above
        mqtt_session( boost::asio::io_service& io_service )
            : socket_( io_service ),
              read_buffer_( std::vector<uint8_t>( MAX_HEADER_LENGTH ) ),
              header_parser_( header_parser( ) )
        {
        }

        void do_read( )
        {
            auto self( shared_from_this( ) );

            socket_.async_read_some( boost::asio::buffer( read_buffer_ ),
                                     [this, self]( boost::system::error_code ec, std::size_t bytes_transferred )
                                     {

            } );
        }

        void do_write( )
        {
        }

        /// Max fixed header length in bytes
        static const size_t MAX_HEADER_LENGTH = 5;
        /// The client socket this session is connected to
        tcp::socket socket_;
        std::vector<uint8_t> read_buffer_;
        header_parser header_parser_;
    };
}
