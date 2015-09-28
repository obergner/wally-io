#include <boost/bind.hpp>

#include "io_wally/app/logging.hpp"
#include "io_wally/mqtt_connection.hpp"
#include "io_wally/mqtt_connection_manager.hpp"

using boost::asio::ip::tcp;

using namespace std;
using namespace io_wally::protocol;
using namespace io_wally::decoder;

namespace io_wally
{
    mqtt_connection::pointer mqtt_connection::create( tcp::socket socket,
                                                      mqtt_connection_manager& session_manager,
                                                      authentication_service& authentication_service )
    {
        return pointer( new mqtt_connection( move( socket ), session_manager, authentication_service ) );
    }

    mqtt_connection::mqtt_connection( tcp::socket socket,
                                      mqtt_connection_manager& session_manager,
                                      authentication_service& authentication_service )
        : id_( nullptr ),
          session_manager_( session_manager ),
          socket_( move( socket ) ),
          authentication_service_( authentication_service )
    {
        return;
    }

    mqtt_connection::~mqtt_connection( )
    {
        if ( id_ )
            delete id_;
        return;
    }

    struct mqtt_connection_id* mqtt_connection::id( ) const
    {
        return id_;
    }

    void mqtt_connection::start( )
    {
        BOOST_LOG_SEV( logger_, lvl::info ) << "START: " << to_string( );
        read_header( );
    }

    void mqtt_connection::stop( )
    {
        // WARNING: Calling to_string() on a closed socket will crash process!
        const string session_desc = to_string( );
        boost::system::error_code ignored_ec;
        socket_.shutdown( socket_.shutdown_both, ignored_ec );
        socket_.close( ignored_ec );
        BOOST_LOG_SEV( logger_, lvl::info ) << "STOPPED: " << session_desc;
    }

    const string mqtt_connection::to_string( ) const
    {
        ostringstream output;
        output << "session[" << socket_ << "]";

        return output.str( );
    }

    void mqtt_connection::read_header( )
    {
        BOOST_LOG_SEV( logger_, lvl::debug ) << "<<< READ: header [bufs:" << read_buffer_.size( ) << "] ...";
        boost::asio::async_read(
            socket_,
            boost::asio::buffer( read_buffer_ ),
            boost::asio::transfer_at_least( 2 ),  // FIXME: This won't work if we are called in a loop
            boost::bind( &mqtt_connection::on_header_data_read,
                         shared_from_this( ),
                         boost::asio::placeholders::error,
                         boost::asio::placeholders::bytes_transferred ) );
    }

    void mqtt_connection::on_header_data_read( const boost::system::error_code& ec, const size_t bytes_transferred )
    {
        if ( ec )
        {
            BOOST_LOG_SEV( logger_, lvl::error ) << "<<< Failed to read header: [ec:" << ec << "|emsg:" << ec.message( )
                                                 << "|bt:" << bytes_transferred << "|bufs:" << read_buffer_.size( )
                                                 << "]";
            if ( ec != boost::asio::error::operation_aborted )
                session_manager_.stop( shared_from_this( ) );
            return;
        }

        try
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "<<< Header data read [bt:" << bytes_transferred
                                                 << "|bufs:" << read_buffer_.size( ) << "]";
            const header_decoder::result<buf_iter> result =
                header_decoder_.decode( read_buffer_.begin( ), read_buffer_.begin( ) + bytes_transferred );
            if ( !result.is_parsing_complete( ) )
            {
                // HIGHLY UNLIKELY: header is at most 5 bytes.
                BOOST_LOG_SEV( logger_, lvl::warning ) << "<<< Header data incomplete - continue";
                read_header( );

                return;
            }
            // Reset header_decoder's internal state
            header_decoder_.reset( );

            BOOST_LOG_SEV( logger_, lvl::info ) << "<<< HEADER [res:" << result.parsed_header( )
                                                << "|bt:" << bytes_transferred << "|bufs:" << read_buffer_.size( )
                                                << "]";
            read_body( result, bytes_transferred );
        }
        catch ( const io_wally::decoder::error::malformed_mqtt_packet& e )
        {
            BOOST_LOG_SEV( logger_, lvl::error )
                << "Malformed control packet header - will stop this session: " << e.what( );
            session_manager_.stop( shared_from_this( ) );
        }
    }

    void mqtt_connection::read_body( const header_decoder::result<buf_iter>& header_parse_result,
                                     const size_t bytes_transferred )
    {
        const uint32_t total_length = header_parse_result.parsed_header( ).total_length( );
        const uint32_t remaining_length = header_parse_result.parsed_header( ).remaining_length( );
        const uint32_t header_length = total_length - remaining_length;

        BOOST_LOG_SEV( logger_, lvl::debug ) << "<<< Reading body [tl:" << total_length << "|rl:" << remaining_length
                                             << "|bt:" << bytes_transferred << "|bufs:" << read_buffer_.size( )
                                             << "] ...";

        if ( bytes_transferred >= total_length )
        {
            // We already received the entire packet. No need to wait for more data.
            on_body_data_read( header_parse_result,
                               boost::system::errc::make_error_code( boost::system::errc::success ),
                               bytes_transferred - header_length );
        }
        else
        {
            // FIXME: This code path has probably never been excercised and needs to be revisited.

            // Resize read buffer to allow for storing the control packet, but do NOT shrink it below
            // its initial default capacity
            read_buffer_.resize( total_length );

            pointer self( shared_from_this( ) );
            boost::asio::async_read(
                socket_,
                boost::asio::buffer( read_buffer_, total_length ),
                boost::asio::transfer_at_least( total_length - bytes_transferred ),
                [self, header_parse_result]( const boost::system::error_code& ec, const size_t bytes_transferred )
                {
                    self->on_body_data_read( header_parse_result, ec, bytes_transferred );
                } );
        }
    }

    void mqtt_connection::on_body_data_read( const header_decoder::result<buf_iter>& header_parse_result,
                                             const boost::system::error_code& ec,
                                             const size_t bytes_transferred )
    {
        if ( ec )
        {
            BOOST_LOG_SEV( logger_, lvl::error ) << "<<< Failed to read body: [ec:" << ec << "|bt:" << bytes_transferred
                                                 << "|bufs:" << read_buffer_.size( ) << "]";
            if ( ec != boost::asio::error::operation_aborted )
                session_manager_.stop( shared_from_this( ) );
            return;
        }

        try
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "<<< Body data read. Decoding ...";
            const unique_ptr<const mqtt_packet> parsed_packet = packet_decoder_.decode(
                header_parse_result.parsed_header( ),
                header_parse_result.consumed_until( ),
                header_parse_result.consumed_until( ) + header_parse_result.parsed_header( ).remaining_length( ) );
            BOOST_LOG_SEV( logger_, lvl::info ) << "<<< DECODED [res:" << *parsed_packet << "|bt:" << bytes_transferred
                                                << "|bufs:" << read_buffer_.size( ) << "]";
            // FIXME: This risks discarding (the first bytes of) another packet that might already have been received!
            read_buffer_.clear( );
            read_buffer_.resize( initial_buffer_capacity );

            dispatch_decoded_packet( *parsed_packet );
        }
        catch ( const io_wally::decoder::error::malformed_mqtt_packet& e )
        {
            BOOST_LOG_SEV( logger_, lvl::error )
                << "<<< Malformed control packet body - will stop this session: " << e.what( );
            session_manager_.stop( shared_from_this( ) );
        }

        return;
    }

    void mqtt_connection::dispatch_decoded_packet( const mqtt_packet& packet )
    {
        BOOST_LOG_SEV( logger_, lvl::debug ) << "--- DISPATCHING: " << packet << " ...";
        switch ( packet.header( ).type( ) )
        {
            case packet::Type::CONNECT:
            {
                const io_wally::protocol::connect& connect = dynamic_cast<const io_wally::protocol::connect&>( packet );
                if ( !authentication_service_.authenticate( socket_.remote_endpoint( ).address( ).to_string( ),
                                                            connect.payload( ).username( ),
                                                            connect.payload( ).password( ) ) )
                {
                    write_packet_and_close_session( connack( false, connect_return_code::BAD_USERNAME_OR_PASSWORD ),
                                                    "Authentication failed" );
                }
                else
                {
                    authenticated = true;
                    write_packet( connack( false, connect_return_code::CONNECTION_ACCEPTED ) );
                }
            }
            break;
            case packet::Type::PINGREQ:
            {
                write_packet( pingresp( ) );
            }
            break;
            case packet::Type::DISCONNECT:
            {
                BOOST_LOG_SEV( logger_, lvl::info ) << "--- Session disconnected by client";
                session_manager_.stop( shared_from_this( ) );
            }
            break;
            default:
                break;
        }
        BOOST_LOG_SEV( logger_, lvl::info ) << "--- DISPATCHED: " << packet;
    }

    void mqtt_connection::write_packet( const mqtt_packet& packet )
    {
        BOOST_LOG_SEV( logger_, lvl::debug ) << ">>> SEND: " << packet << " ...";

        write_buffer_.clear( );
        write_buffer_.resize( packet.header( ).total_length( ) );
        packet_encoder_.encode(
            packet, write_buffer_.begin( ), write_buffer_.begin( ) + packet.header( ).total_length( ) );

        auto self( shared_from_this( ) );
        boost::asio::async_write( socket_,
                                  boost::asio::buffer( write_buffer_, packet.header( ).total_length( ) ),
                                  [self]( const boost::system::error_code& ec, size_t /* bytes_written */ )
                                  {
            if ( ec )
            {
                BOOST_LOG_SEV( self->logger_, lvl::error )
                    << ">>> Failed to send packet - session will be closed: " << ec;
            }
            else
            {
                BOOST_LOG_SEV( self->logger_, lvl::debug ) << ">>> SENT";
                self->read_header( );
            }
        } );
    }

    void mqtt_connection::write_packet_and_close_session( const mqtt_packet& packet, const string& message )
    {
        BOOST_LOG_SEV( logger_, lvl::debug ) << ">>> SEND: " << packet << " - SESSION WILL BE CLOSED: " << message
                                             << " ...";
        write_buffer_.clear( );
        write_buffer_.resize( packet.header( ).total_length( ) );
        packet_encoder_.encode(
            packet, write_buffer_.begin( ), write_buffer_.begin( ) + packet.header( ).total_length( ) );

        auto self( shared_from_this( ) );
        boost::asio::async_write( socket_,
                                  boost::asio::buffer( write_buffer_.data( ), packet.header( ).total_length( ) ),
                                  [self]( const boost::system::error_code& ec, size_t /* bytes_written */ )
                                  {
            if ( ec )
            {
                BOOST_LOG_SEV( self->logger_, lvl::error ) << ">>> Failed to send packet: " << ec;
            }
            else
            {
                BOOST_LOG_SEV( self->logger_, lvl::debug ) << ">>> SENT";
            }
            self->session_manager_.stop( self );
        } );
    }
}
