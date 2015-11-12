#include "io_wally/mqtt_connection.hpp"

#include <cassert>
#include <memory>
#include <chrono>
#include <functional>

#include <boost/system/error_code.hpp>

#include <boost/bind.hpp>

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

    namespace lvl = boost::log::trivial;

    // ---------------------------------------------------------------------------------------------------------------
    // Public
    // ---------------------------------------------------------------------------------------------------------------

    mqtt_connection::ptr mqtt_connection::create( tcp::socket socket,
                                                  mqtt_connection_manager& connection_manager,
                                                  const context& context,
                                                  packetq_t& dispatchq )
    {
        return std::shared_ptr<mqtt_connection>{
            new mqtt_connection{move( socket ), connection_manager, context, dispatchq}};
    }

    mqtt_connection::mqtt_connection( tcp::socket socket,
                                      mqtt_connection_manager& connection_manager,
                                      const context& context,
                                      packetq_t& dispatchq )
        : socket_{move( socket )},
          strand_{socket.get_io_service( )},
          connection_manager_{connection_manager},
          context_{context},
          dispatchq_{dispatchq},
          read_buffer_( context[context::READ_BUFFER_SIZE].as<const size_t>( ) ),
          write_buffer_( context[context::WRITE_BUFFER_SIZE].as<const size_t>( ) ),
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
        auto self = shared_from_this( );
        auto conn_to = chrono::milliseconds{context_[context::CONNECT_TIMEOUT].as<const uint32_t>( )};
        close_on_connection_timeout_.expires_from_now( conn_to );
        close_on_connection_timeout_.async_wait(
            strand_.wrap( [self]( const boost::system::error_code& ec )
                          {
                              if ( !ec )
                              {
                                  auto msg = ostringstream{};
                                  msg << "CONNECTION TIMEOUT EXPIRED after ["
                                      << self->context_[context::CONNECT_TIMEOUT].as<const uint32_t>( )
                                      << "] ms - connection [" << self->socket_ << "] will be closed";
                                  self->connection_close_requested(
                                      msg.str( ), dispatch::disconnect_reason::protocol_violation, ec, lvl::warning );
                              }
                          } ) );

        read_header( );
    }

    void mqtt_connection::send( mqtt_packet::ptr packet )
    {
        write_packet( *packet );
    }

    void mqtt_connection::stop( const string& message, const boost::log::trivial::severity_level log_level )
    {
        BOOST_LOG_SEV( logger_, log_level ) << message;
        auto self = shared_from_this( );
        strand_.get_io_service( ).dispatch( strand_.wrap( [self]( )
                                                          {
                                                              self->connection_manager_.stop( self );
                                                          } ) );
    }

    const string mqtt_connection::to_string( ) const
    {
        auto output = ostringstream{};
        // WARNING: Calling to_string() on a closed socket will crash process!
        if ( socket_.is_open( ) )
            output << "connection[" << socket_ << "]";
        else
            output << "connection[DISCONNECTED/" << ( client_id_ ? *client_id_ : "[NOT AUTHENTICATED]" ) << "]";

        return output.str( );
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Private
    // ---------------------------------------------------------------------------------------------------------------

    void mqtt_connection::do_stop( )
    {
        auto const connection_desc = to_string( );
        auto ignored_ec = boost::system::error_code{};
        socket_.shutdown( socket_.shutdown_both, ignored_ec );
        socket_.close( ignored_ec );

        BOOST_LOG_SEV( logger_, lvl::info ) << "STOPPED: " << connection_desc;
    }

    // Reading incoming messages

    void mqtt_connection::read_header( )
    {
        if ( !socket_.is_open( ) )  // Socket was closed
            return;

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
            on_read_failed( ec, bytes_transferred );
            return;
        }

        try
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "<<< Header data read [bt:" << bytes_transferred
                                                 << "|bufs:" << read_buffer_.size( ) << "]";
            auto const result =
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
        catch ( const error::malformed_mqtt_packet& e )
        {
            connection_close_requested(
                "<<< Malformed control packet header - will close this connection: " + string{e.what( )},
                dispatch::disconnect_reason::protocol_violation );
        }
    }

    void mqtt_connection::read_body( const header_decoder::result<buf_iter>& header_parse_result,
                                     const size_t bytes_transferred )
    {
        auto const total_length = header_parse_result.parsed_header( ).total_length( );
        auto const remaining_length = header_parse_result.parsed_header( ).remaining_length( );
        auto const header_length = total_length - remaining_length;

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
            if ( !socket_.is_open( ) )  // Socket was asynchronously closed
                return;

            // Resize read buffer to allow for storing the control packet, but do NOT shrink it below
            // its initial default capacity
            read_buffer_.resize( total_length );

            auto self = shared_from_this( );
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
            on_read_failed( ec, bytes_transferred );
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
            read_buffer_.resize( context_[context::READ_BUFFER_SIZE].as<const size_t>( ) );

            process_decoded_packet( move( parsed_packet ) );
        }
        catch ( const error::malformed_mqtt_packet& e )
        {
            connection_close_requested(
                "<<< Malformed control packet body - will stop this connection: " + string{e.what( )},
                dispatch::disconnect_reason::protocol_violation );
        }

        return;
    }

    void mqtt_connection::on_read_failed( const boost::system::error_code& ec, const size_t bytes_transferred )
    {
        assert( ec );
        if ( ( ec == boost::asio::error::eof ) || ( ec == boost::asio::error::connection_reset ) )
        {
            // Client disconnected. Could be that he sent a last packet we want to decode.
            BOOST_LOG_SEV( logger_, lvl::info )
                << "<<< EOF/CONNECTION RESET: client disconnected - decoding last received packet ...";
            // TODO: We should at least try to decode a last DISCONNECT - otherwise (if there is no DISCONNECT) this was
            // an unexpected connection loss.
            auto const result =
                header_decoder_.decode( read_buffer_.begin( ), read_buffer_.begin( ) + bytes_transferred );
            if ( !result.is_parsing_complete( ) )
            {
                // HIGHLY UNLIKELY: header is at most 5 bytes.
                BOOST_LOG_SEV( logger_, lvl::warning ) << "<<< Header data incomplete - abort";
            }
            else
            {
                // Reset header_decoder's internal state: not needed since we will close this connection no matter what,
                // but Mum always told me to clean up after yourself ...
                header_decoder_.reset( );

                BOOST_LOG_SEV( logger_, lvl::info ) << "<<< Last received message: " << result.parsed_header( );
            }
            connection_close_requested(
                "<<< Client disconnected", dispatch::disconnect_reason::client_disconnect, ec, lvl::info );
        }
        else if ( ec != boost::asio::error::operation_aborted )  // opration_aborted: regular shutdown sequence
        {
            connection_close_requested(
                "<<< Failed to read header", dispatch::disconnect_reason::network_or_server_failure, ec, lvl::error );
        }
    }

    // Processing and dispatching decoded messages

    void mqtt_connection::process_decoded_packet( shared_ptr<protocol::mqtt_packet> packet )
    {
        BOOST_LOG_SEV( logger_, lvl::debug ) << "--- PROCESSING: " << *packet << " ...";
        switch ( packet->header( ).type( ) )
        {
            case packet::Type::CONNECT:
            {
                process_connect_packet( dynamic_pointer_cast<protocol::connect>( packet ) );
            }
            break;
            case packet::Type::PINGREQ:
            {
                write_packet( pingresp( ) );
                BOOST_LOG_SEV( logger_, lvl::info ) << "--- PROCESSED: " << *packet;
            }
            break;
            case packet::Type::DISCONNECT:
            {
                process_disconnect_packet( dynamic_pointer_cast<protocol::disconnect>( packet ) );
            }
            break;
            case packet::Type::SUBSCRIBE:
            case packet::Type::PUBLISH:
            case packet::Type::PUBACK:
            case packet::Type::PUBREC:
            case packet::Type::PUBCOMP:
            {
                dispatch_packet( packet );
            }
            break;
            case packet::Type::CONNACK:
            case packet::Type::PINGRESP:
            case packet::Type::PUBREL:
            case packet::Type::SUBACK:
            case packet::Type::UNSUBSCRIBE:
            case packet::Type::UNSUBACK:
            case packet::Type::RESERVED1:
            case packet::Type::RESERVED2:
            default:
                assert( false );
                break;
        }
        // Keep us in the loop!
        read_header( );
    }

    void mqtt_connection::process_connect_packet( shared_ptr<protocol::connect> connect )
    {
        if ( client_id_ )
        {
            // [MQTT-3.1.0-2]: If receiving a second CONNECT on an already authenticated connection, that
            // connection MUST be closed
            connection_close_requested(
                "--- [MQTT-3.1.0-2] Received CONNECT on already authenticated "
                "connection - connection will be closed",
                dispatch::disconnect_reason::protocol_violation );
        }
        else if ( !context_.authentication_service( ).authenticate(
                      socket_.remote_endpoint( ).address( ).to_string( ), connect->username( ), connect->password( ) ) )
        {
            write_packet_and_close_connection( connack{false, connect_return_code::BAD_USERNAME_OR_PASSWORD},
                                               "--- Authentication failed",
                                               dispatch::disconnect_reason::authentication_failed );
        }
        else
        {
            close_on_connection_timeout_.cancel( );
            client_id_.emplace( connect->client_id( ) );  // use emplace() to preserve string constness

            if ( connect->keep_alive_secs( ) > 0 )
            {
                keep_alive_ = chrono::seconds{connect->keep_alive_secs( )};
                close_on_keep_alive_timeout( );
            }
            BOOST_LOG_SEV( logger_, lvl::info ) << "--- PROCESSED: " << *connect;

            dispatch_connect_packet( connect );
        }
    }

    void mqtt_connection::dispatch_connect_packet( shared_ptr<protocol::connect> connect )
    {
        BOOST_LOG_SEV( logger_, lvl::debug ) << "--- DISPATCHING: " << *connect << " ...";
        auto connect_container = packet_container_t::contain(
            connect->client_id( ), shared_from_this( ), connect, dispatch::disconnect_reason::client_disconnect );

        auto self = shared_from_this( );
        dispatcher_.async_enq( connect_container,
                               strand_.wrap( [self, connect]( const boost::system::error_code& ec )
                                             {
                                                 self->handle_dispatch_connect_packet( ec, connect );
                                             } ) );
    }

    void mqtt_connection::handle_dispatch_connect_packet( const boost::system::error_code& ec,
                                                          shared_ptr<protocol::connect> connect )
    {
        if ( ec )
        {
            BOOST_LOG_SEV( logger_, lvl::error ) << "--- DISPATCH FAILED: " << *connect << " [ec:" << ec
                                                 << "|emsg:" << ec.message( ) << "]";
            write_packet_and_close_connection( connack{false, connect_return_code::SERVER_UNAVAILABLE},
                                               "--- Dispatching CONNECT failed",
                                               dispatch::disconnect_reason::network_or_server_failure );
        }
        else
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "--- DISPATCHED: " << *connect;
            write_packet( connack{false, connect_return_code::CONNECTION_ACCEPTED} );
        }
    }

    void mqtt_connection::process_disconnect_packet( shared_ptr<protocol::disconnect> disconnect )
    {
        dispatch_disconnect_packet( disconnect );
    }

    void mqtt_connection::dispatch_disconnect_packet( shared_ptr<protocol::disconnect> disconnect,
                                                      const dispatch::disconnect_reason disconnect_reason )
    {
        BOOST_LOG_SEV( logger_, lvl::debug ) << "--- DISPATCHING: " << *disconnect << "[rsn:" << disconnect_reason
                                             << "] ...";
        auto connect_container =
            packet_container_t::contain( *client_id_, shared_from_this( ), disconnect, disconnect_reason );

        auto self = shared_from_this( );
        dispatcher_.async_enq(
            connect_container,
            strand_.wrap( [self, disconnect, disconnect_reason]( const boost::system::error_code& ec )
                          {
                              self->handle_dispatch_disconnect_packet( ec, disconnect, disconnect_reason );
                          } ) );
    }

    void mqtt_connection::handle_dispatch_disconnect_packet( const boost::system::error_code& ec,
                                                             shared_ptr<protocol::disconnect> disconnect,
                                                             const dispatch::disconnect_reason disconnect_reason )
    {
        if ( ec )
        {
            BOOST_LOG_SEV( logger_, lvl::error ) << "--- DISPATCH FAILED: " << *disconnect << " [ec:" << ec
                                                 << "|emsg:" << ec.message( ) << "]";
        }
        else
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "--- DISPATCHED: " << *disconnect << " [rsn:" << disconnect_reason
                                                 << "]";
        }
        auto msg = ostringstream{};
        msg << "--- Connection disconnected: " << disconnect_reason;
        stop( msg.str( ), lvl::info );
    }

    void mqtt_connection::dispatch_packet( shared_ptr<protocol::mqtt_packet> packet )
    {
        BOOST_LOG_SEV( logger_, lvl::debug ) << "--- DISPATCHING: " << *packet << " ...";
        auto subscribe_container = packet_container_t::contain( *client_id_, shared_from_this( ), packet );

        auto self = shared_from_this( );
        dispatcher_.async_enq( subscribe_container,
                               strand_.wrap( [self, packet]( const boost::system::error_code& ec )
                                             {
                                                 self->handle_dispatch_packet( ec, packet );
                                             } ) );
    }

    void mqtt_connection::handle_dispatch_packet( const boost::system::error_code& ec,
                                                  shared_ptr<protocol::mqtt_packet> packet )
    {
        if ( ec )
        {
            BOOST_LOG_SEV( logger_, lvl::error ) << "--- DISPATCH FAILED: " << *packet << " [ec:" << ec
                                                 << "|emsg:" << ec.message( ) << "]";
            connection_close_requested(
                "Dispatching packet failed", dispatch::disconnect_reason::network_or_server_failure, ec, lvl::error );
        }
        else
        {
            BOOST_LOG_SEV( logger_, lvl::debug ) << "--- DISPATCHED: " << *packet;
        }
    }

    // Sending messages

    void mqtt_connection::write_packet( const protocol::mqtt_packet& packet )
    {
        if ( !socket_.is_open( ) )  // Socket was asynchronously closed
            return;

        BOOST_LOG_SEV( logger_, lvl::debug ) << ">>> SEND: " << packet << " ...";

        write_buffer_.clear( );
        write_buffer_.resize( packet.header( ).total_length( ) );
        packet_encoder_.encode(
            packet, write_buffer_.begin( ), write_buffer_.begin( ) + packet.header( ).total_length( ) );

        auto self = shared_from_this( );
        boost::asio::async_write(
            socket_,
            boost::asio::buffer( write_buffer_, packet.header( ).total_length( ) ),
            strand_.wrap( [self]( const boost::system::error_code& ec, size_t /* bytes_written */ )
                          {
                              if ( ec )
                              {
                                  self->connection_close_requested(
                                      "Failed to send packet",
                                      dispatch::disconnect_reason::network_or_server_failure,
                                      ec,
                                      lvl::error );
                              }
                              else
                              {
                                  BOOST_LOG_SEV( self->logger_, lvl::debug ) << ">>> SENT";
                              }
                          } ) );
    }

    void mqtt_connection::write_packet_and_close_connection( const protocol::mqtt_packet& packet,
                                                             const string& message,
                                                             const dispatch::disconnect_reason reason )
    {
        if ( !socket_.is_open( ) )  // Socket was asynchronously closed
            return;

        BOOST_LOG_SEV( logger_, lvl::debug ) << ">>> SEND: " << packet << " - connection will be closed: " << message
                                             << " ...";
        write_buffer_.clear( );
        write_buffer_.resize( packet.header( ).total_length( ) );
        packet_encoder_.encode(
            packet, write_buffer_.begin( ), write_buffer_.begin( ) + packet.header( ).total_length( ) );

        auto self = shared_from_this( );
        boost::asio::async_write(
            socket_,
            boost::asio::buffer( write_buffer_.data( ), packet.header( ).total_length( ) ),
            strand_.wrap( [self, reason]( const boost::system::error_code& ec, size_t /* bytes_written */ )
                          {
                              if ( ec )
                              {
                                  self->connection_close_requested(
                                      ">>> Failed to send packet", reason, ec, lvl::error );
                              }
                              else
                              {
                                  self->connection_close_requested( ">>> SENT", reason, ec, lvl::debug );
                              }
                          } ) );
    }

    // Closing this connection

    void mqtt_connection::close_on_keep_alive_timeout( )
    {
        if ( keep_alive_ )
        {
            auto self = shared_from_this( );
            close_on_keep_alive_timeout_.expires_from_now( *keep_alive_ );
            close_on_connection_timeout_.async_wait( strand_.wrap(
                [self]( const boost::system::error_code& ec )
                {
                    if ( !ec )
                    {
                        auto msg = ostringstream{};
                        msg << "Keep alive timeout expired after [" << ( self->keep_alive_ )->count( ) << "] s";
                        self->connection_close_requested(
                            msg.str( ), dispatch::disconnect_reason::keep_alive_timeout_expired, ec, lvl::warning );
                    }
                } ) );
        }
    }

    void mqtt_connection::connection_close_requested( const std::string& message,
                                                      const dispatch::disconnect_reason reason,
                                                      const boost::system::error_code& ec,
                                                      const boost::log::trivial::severity_level log_level )
    {
        auto err_info = ( ec ? " [ec:" + std::to_string( ec.value( ) ) + "|emsg:" + ec.message( ) + "]" : "" );
        BOOST_LOG_SEV( logger_, log_level ) << "CLOSE REQUESTED (" << reason << "): " << message << err_info
                                            << " - connection will be closed";
        if ( client_id_ )
        {
            // Only "fake" DISCONNECT if we are actually CONNECTed, i.e. we know our client's client_id. Otherwise,
            //
            // - there won't be client_session to close anyway
            // - we don't have a means of identifying any client_session anyway
            //
            auto disconnect = make_shared<protocol::disconnect>( );
            dispatch_disconnect_packet( disconnect, reason );
        }
        else
        {
            stop( message, lvl::error );
        }
    }
}
