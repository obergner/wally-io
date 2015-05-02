#include <boost/bind.hpp>

#include "io_wally/logging.hpp"
#include "io_wally/mqtt_session.hpp"

using boost::asio::ip::tcp;

using namespace io_wally::protocol;
using namespace io_wally::protocol::parser;

namespace io_wally
{
    mqtt_session::pointer mqtt_session::create( tcp::socket socket )
    {
        return pointer( new mqtt_session( std::move( socket ) ) );
    }

    mqtt_session::~mqtt_session( )
    {
        return;
    }

    void mqtt_session::start( )
    {
        BOOST_LOG_TRIVIAL( info ) << "MQTT session started";
        read_header( );
    }

    void mqtt_session::stop( )
    {
        socket_.close( );
        BOOST_LOG_TRIVIAL( info ) << "MQTT session stopped";
    }

    mqtt_session::mqtt_session( tcp::socket socket )
        : socket_( std::move( socket ) ),
          read_buffer_( std::vector<uint8_t>( MAX_HEADER_LENGTH ) ),
          header_parser_( header_parser( ) )
    {
        return;
    }

    void mqtt_session::read_header( )
    {
        BOOST_LOG_TRIVIAL( debug ) << "Reading header ...";
        socket_.async_read_some( boost::asio::buffer( read_buffer_ ),
                                 boost::bind( &mqtt_session::on_header_data_read,
                                              shared_from_this( ),
                                              boost::asio::placeholders::error,
                                              boost::asio::placeholders::bytes_transferred ) );
    }

    void mqtt_session::on_header_data_read( const boost::system::error_code& ec, const size_t bytes_transferred )
    {
        BOOST_LOG_TRIVIAL( debug ) << "Header data read - may not yet be complete";
        if ( ec )
        {
            if ( ec != boost::asio::error::operation_aborted )
                stop( );
            return;
        }

        try
        {
            const header_parser::result<uint8_t*> result =
                header_parser_.parse( read_buffer_.data( ), read_buffer_.data( ) + bytes_transferred );
            if ( !result.is_parsing_complete( ) )
            {
                BOOST_LOG_TRIVIAL( debug ) << "Header data incomplete - continue";
                read_header( );
                return;
            }

            BOOST_LOG_TRIVIAL( info ) << "Header data complete";
            read_body( result );
        }
        catch ( const error::malformed_mqtt_packet& e )
        {
            std::cerr << e.what( ) << std::endl;
            stop( );
        }
    }

    void mqtt_session::read_body( const header_parser::result<uint8_t*>& header_parse_result )
    {
        pointer self( shared_from_this( ) );
        const uint32_t remaining_length = header_parse_result.parsed_header( ).remaining_length( );
        uint8_t* body_start = header_parse_result.consumed_until( );
        BOOST_LOG_TRIVIAL( debug ) << "Reading body (remaining length: " << remaining_length << ") ...";

        read_buffer_.resize( remaining_length + 5 );

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
        BOOST_LOG_TRIVIAL( debug ) << "Body data read. Decoding ...";
        if ( ec )
        {
            if ( ec != boost::asio::error::operation_aborted )
                stop( );
            return;
        }

        const std::unique_ptr<const mqtt_packet> parsed_packet =
            packet_parser_.parse( header_parse_result.parsed_header( ),
                                  header_parse_result.consumed_until( ),
                                  header_parse_result.consumed_until( ) + bytes_transferred );
        BOOST_LOG_TRIVIAL( info ) << "Decoded MQTT control packet";

        return;
    }
}
