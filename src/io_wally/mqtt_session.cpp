#include <boost/bind.hpp>

#include "io_wally/logging.hpp"
#include "io_wally/mqtt_session.hpp"
#include "io_wally/mqtt_session_manager.hpp"

using boost::asio::ip::tcp;

using namespace std;
using namespace io_wally::protocol;
using namespace io_wally::decoder;

namespace io_wally
{
    mqtt_session::pointer mqtt_session::create( tcp::socket socket,
                                                mqtt_session_manager& session_manager,
                                                const authentication_service& authentication_service )
    {
        return pointer( new mqtt_session( move( socket ), session_manager, authentication_service ) );
    }

    mqtt_session::mqtt_session( tcp::socket socket,
                                mqtt_session_manager& session_manager,
                                const authentication_service& authentication_service )
        : id_( nullptr ),
          session_manager_( session_manager ),
          read_buffer_( initial_buffer_capacity ),
          write_buffer_( initial_buffer_capacity ),
          socket_( move( socket ) ),
          header_decoder_( ),
          packet_decoder_( ),
          packet_encoder_( ),
          authentication_service_( authentication_service ),
          logger_( keywords::channel = "session", keywords::severity = lvl::trace )
    {
        return;
    }

    mqtt_session::~mqtt_session( )
    {
        if ( id_ )
            delete id_;
        return;
    }

    struct mqtt_session_id* mqtt_session::id( ) const
    {
        return id_;
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

    const string mqtt_session::to_string( ) const
    {
        ostringstream output;
        output << "session[" << socket_ << "]";

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
            const header_decoder::result<buf_iter> result =
                header_decoder_.decode( read_buffer_.begin( ), read_buffer_.begin( ) + bytes_transferred );
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
        catch ( const io_wally::decoder::error::malformed_mqtt_packet& e )
        {
            BOOST_LOG_SEV( logger_, lvl::error )
                << "Malformed control packet header - will stop this session: " << e.what( );
            session_manager_.stop( shared_from_this( ) );
        }
    }

    void mqtt_session::read_body( const header_decoder::result<buf_iter>& header_parse_result,
                                  const size_t bytes_transferred )
    {
        const uint32_t remaining_length = header_parse_result.parsed_header( ).remaining_length( );
        buf_iter body_start = header_parse_result.consumed_until( );
        const size_t header_length = body_start - read_buffer_.begin( );

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
                boost::asio::buffer( read_buffer_, remaining_length ),
                [this, self, header_parse_result]( const boost::system::error_code& ec, const size_t bytes_transferred )
                {
                    on_body_data_read( header_parse_result, ec, bytes_transferred );
                } );
        }
    }

    void mqtt_session::on_body_data_read( const header_decoder::result<buf_iter>& header_parse_result,
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
            const unique_ptr<const mqtt_packet> parsed_packet =
                packet_decoder_.decode( header_parse_result.parsed_header( ),
                                        header_parse_result.consumed_until( ),
                                        header_parse_result.consumed_until( ) + bytes_transferred );
            BOOST_LOG_SEV( logger_, lvl::info ) << "DECODED: " << *parsed_packet;

            // dispatch_decoded_packet( *parsed_packet );
        }
        catch ( const io_wally::decoder::error::malformed_mqtt_packet& e )
        {
            BOOST_LOG_SEV( logger_, lvl::error )
                << "Malformed control packet body - will stop this session: " << e.what( );
            session_manager_.stop( shared_from_this( ) );
        }

        return;
    }

    void mqtt_session::dispatch_decoded_packet( const mqtt_packet& packet )
    {
        BOOST_LOG_SEV( logger_, lvl::debug ) << "DISPATCHING: " << packet;
        switch ( packet.header( ).type( ) )
        {
            case packet::Type::CONNECT:
            {
                const io_wally::protocol::connect& connect = dynamic_cast<const io_wally::protocol::connect&>( packet );
                if ( !authentication_service_( socket_.remote_endpoint( ).address( ).to_string( ),
                                               connect.payload( ).username( ),
                                               connect.payload( ).password( ) ) )
                {
                    write_packet_and_close_session( connack( false, connect_return_code::BAD_USERNAME_OR_PASSWORD ),
                                                    "Authentication failed" );
                }
                else
                {
                    write_packet( connack( false, connect_return_code::CONNECTION_ACCEPTED ) );
                }
            }
            break;
            default:
                break;
        }
        BOOST_LOG_SEV( logger_, lvl::info ) << "DISPATCHED: " << packet;
    }

    void mqtt_session::write_packet( const mqtt_packet& packet )
    {
        BOOST_LOG_SEV( logger_, lvl::debug ) << "Sending packet " << packet << " ...";
        packet_encoder_.encode(
            packet, write_buffer_.begin( ), write_buffer_.begin( ) + packet.header( ).max_total_packet_length( ) );

        auto self( shared_from_this( ) );
        boost::asio::async_write(
            socket_,
            boost::asio::buffer( write_buffer_.data( ), write_buffer_.size( ) ),
            [this, self, &packet]( const boost::system::error_code& ec, size_t /* bytes_written */ )
            {
                write_buffer_.clear( );
                if ( ec )
                {
                    BOOST_LOG_SEV( logger_, lvl::error ) << "Failed to send " << packet
                                                         << " - session will be closed: " << ec;
                }
                else
                {
                    BOOST_LOG_SEV( logger_, lvl::debug ) << "Sent packet " << packet;
                }
            } );
    }

    void mqtt_session::write_packet_and_close_session( const mqtt_packet& packet, const string& message )
    {
        BOOST_LOG_SEV( logger_, lvl::debug ) << "Sending packet " << packet << " - session will be closed: " << message
                                             << " ...";
        packet_encoder_.encode(
            packet, write_buffer_.begin( ), write_buffer_.begin( ) + packet.header( ).max_total_packet_length( ) );

        auto self( shared_from_this( ) );
        boost::asio::async_write(
            socket_,
            boost::asio::buffer( write_buffer_.data( ), write_buffer_.size( ) ),
            [this, self, &packet]( const boost::system::error_code& ec, size_t /* bytes_written */ )
            {
                write_buffer_.clear( );
                if ( ec )
                {
                    BOOST_LOG_SEV( logger_, lvl::error ) << "Failed to send " << packet << ": " << ec;
                }
                else
                {
                    BOOST_LOG_SEV( logger_, lvl::debug ) << "Sent packet " << packet;
                }
                session_manager_.stop( self );
            } );
    }
}
