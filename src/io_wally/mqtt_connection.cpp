#include "io_wally/mqtt_connection.hpp"

#include <boost/bind.hpp>

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <boost/asio.hpp>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/error/protocol.hpp"
#include "io_wally/logging_support.hpp"
#include "io_wally/mqtt_connection_manager.hpp"

namespace io_wally
{
    using boost::asio::ip::tcp;

    using namespace std;
    using namespace io_wally::protocol;
    using namespace io_wally::decoder;

    mqtt_connection::ptr mqtt_connection::create( tcp::socket socket,
                                                  mqtt_connection_manager& session_manager,
                                                  const context& context )
    {
        return mqtt_connection::ptr( new mqtt_connection( move( socket ), session_manager, context ) );
    }

    mqtt_connection::mqtt_connection( tcp::socket socket,
                                      mqtt_connection_manager& session_manager,
                                      const context& context )
        : socket_{move( socket )},
          strand_{socket.get_io_service( )},
          session_manager_{session_manager},
          context_{context},
          read_buffer_( context.options( )[context::READ_BUFFER_SIZE].as<const size_t>( ) ),
          write_buffer_( context.options( )[context::WRITE_BUFFER_SIZE].as<const size_t>( ) ),
          close_on_connection_timeout_{socket.get_io_service( )},
          close_on_keep_alive_timeout_{socket.get_io_service( )}
    {
        return;
    }

    void mqtt_connection::start( )
    {
        BOOST_LOG_SEV( logger_, lvl::info ) << "START: " << to_string( );

        // Start deadline timer that will close this connection if connect timeout expires without receiving CONNECT
        // request
        auto self( shared_from_this( ) );
        close_on_connection_timeout_.expires_from_now(
            boost::posix_time::milliseconds( context_.options( )[context::CONNECT_TIMEOUT].as<const uint32_t>( ) ) );
        close_on_connection_timeout_.async_wait(
            strand_.wrap( [self]( const boost::system::error_code& /* ec */ )
                          {
                              BOOST_LOG_SEV( self->logger_, lvl::warning )
                                  << "CONNECTION TIMEOUT EXPIRED after ["
                                  << self->context_.options( )[context::CONNECT_TIMEOUT].as<const uint32_t>( )
                                  << "] ms - connection [" << self->socket_ << "] will be closed";
                              self->stop( );
                          } ) );

        read_header( );
    }

    void mqtt_connection::stop( )
    {
        // WARNING: Calling to_string() on a closed socket will crash process!
        auto session_desc = to_string( );
        auto ignored_ec = boost::system::error_code{};
        socket_.shutdown( socket_.shutdown_both, ignored_ec );
        socket_.close( ignored_ec );
        BOOST_LOG_SEV( logger_, lvl::info ) << "STOPPED: " << session_desc;
    }

    const string mqtt_connection::to_string( ) const
    {
        auto output = ostringstream{};
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
            strand_.wrap( boost::bind( &mqtt_connection::on_header_data_read,
                                       shared_from_this( ),
                                       boost::asio::placeholders::error,
                                       boost::asio::placeholders::bytes_transferred ) ) );
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
            auto result = header_decoder_.decode( read_buffer_.begin( ), read_buffer_.begin( ) + bytes_transferred );
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
        catch ( const error::malformed_mqtt_packet& e )
        {
            BOOST_LOG_SEV( logger_, lvl::error )
                << "Malformed control packet header - will stop this session: " << e.what( );
            session_manager_.stop( shared_from_this( ) );
        }
    }

    void mqtt_connection::read_body( const header_decoder::result<buf_iter>& header_parse_result,
                                     const size_t bytes_transferred )
    {
        auto total_length = header_parse_result.parsed_header( ).total_length( );
        auto remaining_length = header_parse_result.parsed_header( ).remaining_length( );
        auto header_length = total_length - remaining_length;

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

            auto self( shared_from_this( ) );
            boost::asio::async_read(
                socket_,
                boost::asio::buffer( read_buffer_, total_length ),
                boost::asio::transfer_at_least( total_length - bytes_transferred ),
                strand_.wrap(
                    [self, header_parse_result]( const boost::system::error_code& ec, const size_t bytes_transferred )
                    {
                        self->on_body_data_read( header_parse_result, ec, bytes_transferred );
                    } ) );
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
            // We received a packet, so let's cancel keep alive timer
            close_on_keep_alive_timeout_.cancel( );

            auto parsed_packet = packet_decoder_.decode(
                header_parse_result.parsed_header( ),
                header_parse_result.consumed_until( ),
                header_parse_result.consumed_until( ) + header_parse_result.parsed_header( ).remaining_length( ) );
            BOOST_LOG_SEV( logger_, lvl::info ) << "<<< DECODED [res:" << *parsed_packet << "|bt:" << bytes_transferred
                                                << "|bufs:" << read_buffer_.size( ) << "]";
            // FIXME: This risks discarding (the first bytes of) another packet that might already have been received!
            read_buffer_.clear( );
            read_buffer_.resize( context_.options( )[context::READ_BUFFER_SIZE].as<const size_t>( ) );

            dispatch_decoded_packet( *parsed_packet );
        }
        catch ( const error::malformed_mqtt_packet& e )
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
                auto connect = dynamic_cast<const protocol::connect&>( packet );
                if ( authenticated )
                {
                    // [MQTT-3.1.0-2]: If receiving a second CONNECT on an already authenticated connection, that
                    // connection MUST be closed
                    BOOST_LOG_SEV( logger_, lvl::error )
                        << "--- [MQTT-3.1.0-2] Received CONNECT on already authenticated "
                           "connection - connection will be closed";
                    session_manager_.stop( shared_from_this( ) );
                }
                else if ( !context_.authentication_service( ).authenticate(
                              socket_.remote_endpoint( ).address( ).to_string( ),
                              connect.username( ),
                              connect.password( ) ) )
                {
                    write_packet_and_close_session( connack( false, connect_return_code::BAD_USERNAME_OR_PASSWORD ),
                                                    "--- Authentication failed" );
                }
                else
                {
                    close_on_connection_timeout_.cancel( );
                    authenticated = true;

                    if ( connect.keep_alive_secs( ) > 0 )
                    {
                        keep_alive_ = boost::posix_time::seconds( connect.keep_alive_secs( ) );
                        close_on_keep_alive_timeout( );
                    }

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
            case packet::Type::CONNACK:
            case packet::Type::PINGRESP:
            case packet::Type::PUBLISH:
            case packet::Type::PUBACK:
            case packet::Type::PUBREL:
            case packet::Type::PUBREC:
            case packet::Type::PUBCOMP:
            case packet::Type::SUBSCRIBE:
            case packet::Type::SUBACK:
            case packet::Type::UNSUBSCRIBE:
            case packet::Type::UNSUBACK:
            case packet::Type::RESERVED1:
            case packet::Type::RESERVED2:
            default:
                assert( false );
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
        boost::asio::async_write(
            socket_,
            boost::asio::buffer( write_buffer_, packet.header( ).total_length( ) ),
            strand_.wrap( [self]( const boost::system::error_code& ec, size_t /* bytes_written */ )
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
                          } ) );
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
        boost::asio::async_write(
            socket_,
            boost::asio::buffer( write_buffer_.data( ), packet.header( ).total_length( ) ),
            strand_.wrap( [self]( const boost::system::error_code& ec, size_t /* bytes_written */ )
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
                          } ) );
    }

    void mqtt_connection::close_on_keep_alive_timeout( )
    {
        if ( keep_alive_ )
        {
            auto self( shared_from_this( ) );
            close_on_keep_alive_timeout_.expires_from_now( *keep_alive_ );
            close_on_connection_timeout_.async_wait( strand_.wrap( [self]( const boost::system::error_code& /* ec */ )
                                                                   {
                                                                       BOOST_LOG_SEV( self->logger_, lvl::warning )
                                                                           << "KEEP ALIVE TIMEOUT EXPIRED after ["
                                                                           << ( self->keep_alive_ )->total_seconds( )
                                                                           << "] s - connection [" << self->socket_
                                                                           << "] will be closed";
                                                                       self->stop( );
                                                                   } ) );
        }
    }
}
