#include <boost/bind.hpp>

#include "io_wally/logging.hpp"
#include "io_wally/mqtt_session.hpp"
#include "io_wally/mqtt_session_manager.hpp"

using boost::asio::ip::tcp;

using namespace io_wally::protocol;
using namespace io_wally::protocol::parser;

namespace io_wally
{
    mqtt_session::pointer mqtt_session::create( tcp::socket socket, mqtt_session_manager& session_manager )
    {
        return pointer( new mqtt_session( std::move( socket ), session_manager ) );
    }

    mqtt_session::mqtt_session( tcp::socket socket, mqtt_session_manager& session_manager )
        : session_manager_( session_manager ),
          read_buffer_( std::vector<uint8_t>( initial_buffer_capacity ) ),
          socket_( std::move( socket ) ),
          header_parser_( ),
          packet_parser_( ),
          logger_( keywords::channel = "session", keywords::severity = lvl::trace )
    {
        return;
    }

    mqtt_session::~mqtt_session( )
    {
        return;
    }

    void mqtt_session::start( )
    {
        BOOST_LOG_SEV( logger_, lvl::info ) << "START: MQTT session " << socket_;
        read_header( );
    }

    void mqtt_session::stop( )
    {
        socket_.close( );
        BOOST_LOG_SEV( logger_, lvl::info ) << "STOP: MQTT session " << socket_;
    }

    const std::string mqtt_session::to_string( ) const
    {
        std::ostringstream output;
        output << "session[" << socket_;

        return output.str( );
    }

    void mqtt_session::read_header( )
    {
        BOOST_LOG_SEV( logger_, lvl::debug ) << "READ: header ...";
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
            BOOST_LOG_SEV( logger_, lvl::error ) << "Failed to read header: [error_code:" << ec << "]";
            if ( ec != boost::asio::error::operation_aborted )
                session_manager_.stop( shared_from_this( ) );
            return;
        }

        try
        {
            const header_parser::result<uint8_t*> result =
                header_parser_.parse( read_buffer_.data( ), read_buffer_.data( ) + bytes_transferred );
            if ( !result.is_parsing_complete( ) )
            {
                // HIGHLY UNLIKELY: header is at most 5 bytes.
                BOOST_LOG_SEV( logger_, lvl::warning ) << "Header data incomplete - continue";
                read_header( );
                return;
            }

            BOOST_LOG_SEV( logger_, lvl::info ) << "RCVD: " << result.parsed_header( );
            read_body( result, bytes_transferred );
        }
        catch ( const error::malformed_mqtt_packet& e )
        {
            BOOST_LOG_SEV( logger_, lvl::error )
                << "Malformed control packet header - will stop this session: " << e.what( );
            session_manager_.stop( shared_from_this( ) );
        }
    }

    void mqtt_session::read_body( const header_parser::result<uint8_t*>& header_parse_result,
                                  const size_t bytes_transferred )
    {
        const uint32_t remaining_length = header_parse_result.parsed_header( ).remaining_length( );
        uint8_t* body_start = header_parse_result.consumed_until( );
        const size_t header_length = body_start - &read_buffer_.front( );

        BOOST_LOG_SEV( logger_, lvl::debug ) << "Reading body (remaining length: " << remaining_length << ") ...";

        if ( bytes_transferred >= ( header_length + remaining_length ) )
        {
            // We already received the entire packet. No need to wait for more data.
            on_body_data_read( header_parse_result,
                               boost::system::errc::make_error_code( boost::system::errc::success ),
                               bytes_transferred - header_length );
        }
        else
        {
            // Resize read buffer to allow for storing the control packet, but do NOT shrink it below
            // its initial default capacity
            if ( remaining_length + header_length > initial_buffer_capacity )
                read_buffer_.resize( remaining_length + header_length );

            pointer self( shared_from_this( ) );
            boost::asio::async_read(
                socket_,
                boost::asio::buffer( body_start, remaining_length ),
                [this, self, header_parse_result]( const boost::system::error_code& ec, const size_t bytes_transferred )
                {
                    on_body_data_read( header_parse_result, ec, bytes_transferred );
                } );
        }
    }

    void mqtt_session::on_body_data_read( const header_parser::result<uint8_t*>& header_parse_result,
                                          const boost::system::error_code& ec,
                                          const size_t bytes_transferred )
    {
        if ( ec )
        {
            BOOST_LOG_SEV( logger_, lvl::error ) << "Failed to read body: [error_code:" << ec << "]";
            if ( ec != boost::asio::error::operation_aborted )
                session_manager_.stop( shared_from_this( ) );
            return;
        }

        try
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "Body data read. Decoding ...";
            const std::unique_ptr<const mqtt_packet> parsed_packet =
                packet_parser_.parse( header_parse_result.parsed_header( ),
                                      header_parse_result.consumed_until( ),
                                      header_parse_result.consumed_until( ) + bytes_transferred );
            BOOST_LOG_SEV( logger_, lvl::info ) << "DECODED: " << *parsed_packet;
        }
        catch ( const error::malformed_mqtt_packet& e )
        {
            BOOST_LOG_SEV( logger_, lvl::error )
                << "Malformed control packet body - will stop this session: " << e.what( );
            session_manager_.stop( shared_from_this( ) );
        }

        return;
    }
}
