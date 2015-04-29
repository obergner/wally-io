#include <boost/bind.hpp>

#include "io_wally/mqtt_session.hpp"

using boost::asio::ip::tcp;

using namespace io_wally::protocol;
using namespace io_wally::protocol::parser;

namespace io_wally
{
    mqtt_session::pointer mqtt_session::create( boost::asio::io_service& io_service )
    {
        return pointer( new mqtt_session( io_service ) );
    }

    mqtt_session::~mqtt_session( )
    {
        return;
    }

    void mqtt_session::start( )
    {
        read_header( );
    }

    void mqtt_session::stop( )
    {
        socket_.close( );
    }

    mqtt_session::mqtt_session( boost::asio::io_service& io_service )
        : socket_( io_service ),
          read_buffer_( std::vector<uint8_t>( MAX_HEADER_LENGTH ) ),
          header_parser_( header_parser( ) )
    {
        return;
    }

    void mqtt_session::read_header( )
    {
        socket_.async_read_some( boost::asio::buffer( read_buffer_ ),
                                 boost::bind( &mqtt_session::on_header_data_read,
                                              shared_from_this( ),
                                              boost::asio::placeholders::error,
                                              boost::asio::placeholders::bytes_transferred ) );
    }

    void mqtt_session::on_header_data_read( const boost::system::error_code& ec, const size_t bytes_transferred )
    {
        if ( ec )
        {
            if ( ec != boost::asio::error::operation_aborted )
                stop( );
            return;
        }

        const header_parser::result<uint8_t*> result =
            header_parser_.parse( read_buffer_.data( ), read_buffer_.data( ) + bytes_transferred );
        if ( result.is_input_malformed( ) )
        {
            stop( );
            return;
        }

        if ( !result.is_parsing_complete( ) )
        {
            read_header( );
            return;
        }

        read_body( result );
    }

    void mqtt_session::read_body( const header_parser::result<uint8_t*>& header_parse_result )
    {
        pointer self( shared_from_this( ) );
        const uint32_t remaining_length = header_parse_result.parsed_header( ).remaining_length( );
        uint8_t* body_start = header_parse_result.consumed_until( );

        boost::asio::async_read(
            socket_,
            boost::asio::buffer( body_start, remaining_length ),
            [this, self, header_parse_result]( const boost::system::error_code& ec, const size_t bytes_transferred )
            {
                on_body_data_read( header_parse_result, ec, bytes_transferred );
            } );
    }

    void mqtt_session::on_body_data_read( const header_parser::result<uint8_t*>& header_parse_result,
                                          const boost::system::error_code& ec,
                                          const size_t bytes_transferred )
    {
        return;
    }
}
