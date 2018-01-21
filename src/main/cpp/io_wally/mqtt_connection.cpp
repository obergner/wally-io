#include "io_wally/mqtt_connection.hpp"

#include <cassert>
#include <chrono>
#include <functional>
#include <memory>
#include <system_error>

#include <asio.hpp>

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

#include "io_wally/error/protocol.hpp"
#include "io_wally/logging_support.hpp"
#include "io_wally/mqtt_connection_manager.hpp"

#include "io_wally/dispatch/dispatcher.hpp"

namespace io_wally
{
    using ::asio::ip::tcp;

    using namespace std;
    using namespace io_wally::protocol;
    using namespace io_wally::decoder;

    namespace lvl = boost::log::trivial;

    // ---------------------------------------------------------------------------------------------------------------
    // Public/static
    // ---------------------------------------------------------------------------------------------------------------

    mqtt_connection::ptr mqtt_connection::create( tcp::socket socket,
                                                  mqtt_connection_manager& connection_manager,
                                                  const context& context,
                                                  dispatch::dispatcher& dispatcher )
    {
        return std::shared_ptr<mqtt_connection>{
            new mqtt_connection{move( socket ), connection_manager, context, dispatcher}};
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Private/static
    // ---------------------------------------------------------------------------------------------------------------

    const std::string mqtt_connection::endpoint_description( const ::asio::ip::tcp::socket& socket )
    {
        if ( socket.is_open( ) )
        {
            return "connection/" + socket.remote_endpoint( ).address( ).to_string( ) + ":" +
                   std::to_string( socket.remote_endpoint( ).port( ) );
        }
        else
        {
            return "connection/DISCONNECTED";
        }
    }

    const std::string mqtt_connection::connection_description( const ::asio::ip::tcp::socket& socket,
                                                               const std::string& client_id )
    {
        if ( socket.is_open( ) )
        {
            return "connection/" + socket.remote_endpoint( ).address( ).to_string( ) + ":" +
                   std::to_string( socket.remote_endpoint( ).port( ) ) + "/" + client_id;
        }
        else
        {
            return "connection/DISCONNECTED/" + client_id;
        }
    }

    mqtt_connection::mqtt_connection( tcp::socket socket,
                                      mqtt_connection_manager& connection_manager,
                                      const context& context,
                                      dispatch::dispatcher& dispatcher )
        : description_{connection_description( socket )},
          socket_{move( socket )},
          strand_{socket.get_io_service( )},
          connection_manager_{connection_manager},
          context_{context},
          dispatcher_{dispatcher},
          read_buffer_( context[context::READ_BUFFER_SIZE].as<const size_t>( ) ),
          write_buffer_( context[context::WRITE_BUFFER_SIZE].as<const size_t>( ) ),
          close_on_connection_timeout_{socket.get_io_service( )},
          close_on_keep_alive_timeout_{socket.get_io_service( )}
    {
        return;
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Public
    // ---------------------------------------------------------------------------------------------------------------

    void mqtt_connection::start( )
    {
        BOOST_LOG_SEV( logger_, lvl::info ) << "START: " << *this;

        close_on_connect_timeout( );

        read_frame( );
    }

    void mqtt_connection::send( mqtt_packet::ptr packet )
    {
        write_packet( *packet );
    }

    void mqtt_connection::stop( const string& message, const boost::log::trivial::severity_level log_level )
    {
        BOOST_LOG_SEV( logger_, log_level ) << message;
        auto self = shared_from_this( );
        strand_.get_io_service( ).dispatch( strand_.wrap( [self]( ) { self->connection_manager_.stop( self ); } ) );
    }

    // ---------------------------------------------------------------------------------------------------------------
    // Private
    // ---------------------------------------------------------------------------------------------------------------

    void mqtt_connection::do_stop( )
    {
        close_on_connection_timeout_.cancel( );
        close_on_keep_alive_timeout_.cancel( );

        auto ignored_ec = std::error_code{};
        socket_.shutdown( socket_.shutdown_both, ignored_ec );
        socket_.close( ignored_ec );

        BOOST_LOG_SEV( logger_, lvl::info ) << "STOPPED: " << *this;
    }

    // Deal with connect timeout

    void mqtt_connection::close_on_connect_timeout( )
    {
        // Start deadline timer that will close this connection if connect timeout expires without receiving CONNECT
        // request
        auto self = shared_from_this( );
        auto conn_to = chrono::milliseconds{context_[context::CONNECT_TIMEOUT].as<const uint32_t>( )};
        close_on_connection_timeout_.expires_from_now( conn_to );
        close_on_connection_timeout_.async_wait( strand_.wrap( [self]( const std::error_code& ec ) {
            if ( !ec )
            {
                auto msg = ostringstream{};
                msg << "CONNECTION TIMEOUT EXPIRED after ["
                    << self->context_[context::CONNECT_TIMEOUT].as<const uint32_t>( ) << "] ms";
                self->connection_close_requested( msg.str( ), dispatch::disconnect_reason::protocol_violation, ec,
                                                  lvl::warning );
            }
        } ) );
    }

    // Reading incoming messages

    void mqtt_connection::read_frame( )
    {
        if ( !socket_.is_open( ) )  // Socket was closed
            return;

        BOOST_LOG_SEV( logger_, lvl::debug ) << "<<< READ: next frame ...";

        auto self = shared_from_this( );
        ::asio::async_read( socket_, ::asio::buffer( read_buffer_ ),
                            [self]( const std::error_code& ec, const size_t bytes_transferred ) -> size_t {
                                return self->frame_reader_( ec, bytes_transferred );
                            },
                            strand_.wrap( std::bind( &mqtt_connection::on_frame_read, shared_from_this( ),
                                                     std::placeholders::_1, std::placeholders::_2 ) ) );
    }

    void mqtt_connection::on_frame_read( const std::error_code& ec, const size_t bytes_transferred )
    {
        // We received a packet, so let's cancel keep alive timer
        close_on_keep_alive_timeout_.cancel( );
        if ( ec )
        {
            on_read_failed( ec, bytes_transferred );
            return;
        }

        decode_packet( bytes_transferred );
    }

    void mqtt_connection::decode_packet( const size_t bytes_transferred )
    {
        try
        {
            const std::optional<const frame> frame = frame_reader_.get_frame( );
            assert( frame );
            auto parsed_packet = packet_decoder_.decode( *frame );
            BOOST_LOG_SEV( logger_, lvl::info )
                << "<<< DECODED [res:" << *parsed_packet << "|bytes:" << bytes_transferred << "]";

            frame_reader_.reset( );
            // TODO: Think about better resizing strategy - maybe using a max buffer capacity
            read_buffer_.resize( context_[context::READ_BUFFER_SIZE].as<const size_t>( ) );

            process_decoded_packet( move( parsed_packet ) );
        }
        catch ( const error::malformed_mqtt_packet& e )
        {
            connection_close_requested( "<<< Malformed control packet header: " + string{e.what( )},
                                        dispatch::disconnect_reason::protocol_violation );
        }
    }

    void mqtt_connection::on_read_failed( const std::error_code& ec, const size_t bytes_transferred )
    {
        assert( ec );
        // Reset header_decoder's internal state: not needed since we will close this connection no matter what,
        // but Mum always told me to clean up after yourself ...
        if ( ec != ::asio::error::operation_aborted )  // opration_aborted: regular shutdown sequence
        {
            auto msg = ostringstream{};
            msg << "<<< NETWORK ERROR (" << ec.message( ) << ") after [" << bytes_transferred << "] bytes transferred";
            connection_close_requested( msg.str( ), dispatch::disconnect_reason::network_or_server_failure, ec,
                                        lvl::error );
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
                // Keep us in the loop!
                read_frame( );
            }
            break;
            case packet::Type::PINGREQ:
            {
                write_packet( pingresp( ) );
                BOOST_LOG_SEV( logger_, lvl::info ) << "--- PROCESSED: " << *packet;
                // Keep us in the loop!
                read_frame( );
            }
            break;
            case packet::Type::DISCONNECT:
            {
                process_disconnect_packet( dynamic_pointer_cast<protocol::disconnect>( packet ) );
                // Terminate loop
            }
            break;
            case packet::Type::SUBSCRIBE:
            case packet::Type::UNSUBSCRIBE:
            case packet::Type::PUBLISH:
            case packet::Type::PUBACK:
            case packet::Type::PUBREC:
            case packet::Type::PUBREL:
            case packet::Type::PUBCOMP:
            {
                dispatch_packet( packet );
                // Keep us in the loop!
                read_frame( );
            }
            break;
            case packet::Type::CONNACK:
            case packet::Type::PINGRESP:
            case packet::Type::SUBACK:
            case packet::Type::UNSUBACK:
            case packet::Type::RESERVED1:
            case packet::Type::RESERVED2:
            default:
                assert( false );
                break;
        }
    }

    void mqtt_connection::process_connect_packet( shared_ptr<protocol::connect> connect )
    {
        if ( client_id_ )
        {
            // [MQTT-3.1.0-2]: If receiving a second CONNECT on an already authenticated connection, that
            // connection MUST be closed
            connection_close_requested( "--- [MQTT-3.1.0-2] Received CONNECT on already authenticated connection",
                                        dispatch::disconnect_reason::protocol_violation );
        }
        // TODO: Calling socket_.remote_endpoint() is not safe since we can be disconnected at any time
        else if ( !context_.authentication_service( ).authenticate( socket_.remote_endpoint( ).address( ).to_string( ),
                                                                    connect->username( ), connect->password( ) ) )
        {
            write_packet_and_close_connection( connack{false, connect_return_code::BAD_USERNAME_OR_PASSWORD},
                                               "--- Authentication failed",
                                               dispatch::disconnect_reason::authentication_failed );
        }
        else
        {
            close_on_connection_timeout_.cancel( );
            client_id_.emplace( connect->client_id( ) );  // use emplace() to preserve string constness
            description_ = connection_description( socket_, *client_id_ );

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
        auto connect_container = packet_container_t::contain( connect->client_id( ), shared_from_this( ), connect,
                                                              dispatch::disconnect_reason::client_disconnect );
        dispatcher_.handle_packet_received( connect_container );
        BOOST_LOG_SEV( logger_, lvl::debug ) << "--- DISPATCHED: " << *connect;
        write_packet( connack{false, connect_return_code::CONNECTION_ACCEPTED} );
    }

    void mqtt_connection::process_disconnect_packet( shared_ptr<protocol::disconnect> disconnect )
    {
        dispatch_disconnect_packet( disconnect );
    }

    void mqtt_connection::dispatch_disconnect_packet( shared_ptr<protocol::disconnect> disconnect,
                                                      const dispatch::disconnect_reason disconnect_reason )
    {
        BOOST_LOG_SEV( logger_, lvl::debug )
            << "--- DISPATCHING: " << *disconnect << "[rsn:" << disconnect_reason << "] ...";
        auto disconnect_container =
            packet_container_t::contain( *client_id_, shared_from_this( ), disconnect, disconnect_reason );
        dispatcher_.handle_packet_received( disconnect_container );
        BOOST_LOG_SEV( logger_, lvl::debug )
            << "--- DISPATCHED: " << *disconnect << " [rsn:" << disconnect_reason << "]";

        auto msg = ostringstream{};
        msg << "--- Connection disconnected: " << disconnect_reason;
        stop( msg.str( ), lvl::info );
    }

    void mqtt_connection::dispatch_packet( shared_ptr<protocol::mqtt_packet> packet )
    {
        BOOST_LOG_SEV( logger_, lvl::debug ) << "--- DISPATCHING: " << *packet << " ...";
        auto subscribe_container = packet_container_t::contain( *client_id_, shared_from_this( ), packet );
        dispatcher_.handle_packet_received( subscribe_container );
        BOOST_LOG_SEV( logger_, lvl::debug ) << "--- DISPATCHED: " << *packet;
    }

    // Sending messages

    void mqtt_connection::write_packet( const protocol::mqtt_packet& packet )
    {
        if ( !socket_.is_open( ) )  // Socket was asynchronously closed
            return;

        BOOST_LOG_SEV( logger_, lvl::debug ) << ">>> SEND: " << packet << " ...";

        write_buffer_.clear( );
        write_buffer_.resize( packet.header( ).total_length( ) );
        packet_encoder_.encode( packet, write_buffer_.begin( ),
                                write_buffer_.begin( ) + packet.header( ).total_length( ) );

        auto self = shared_from_this( );
        ::asio::async_write( socket_, ::asio::buffer( write_buffer_, packet.header( ).total_length( ) ),
                             strand_.wrap( [self]( const std::error_code& ec, size_t /* bytes_written */ ) {
                                 if ( ec )
                                 {
                                     self->connection_close_requested(
                                         "Failed to send packet",
                                         dispatch::disconnect_reason::network_or_server_failure, ec, lvl::error );
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

        BOOST_LOG_SEV( logger_, lvl::debug ) << ">>> SEND: " << packet << " - " << message << " ...";
        write_buffer_.clear( );
        write_buffer_.resize( packet.header( ).total_length( ) );
        packet_encoder_.encode( packet, write_buffer_.begin( ),
                                write_buffer_.begin( ) + packet.header( ).total_length( ) );

        auto self = shared_from_this( );
        ::asio::async_write( socket_, ::asio::buffer( write_buffer_.data( ), packet.header( ).total_length( ) ),
                             strand_.wrap( [self, reason]( const std::error_code& ec, size_t /* bytes_written */ ) {
                                 if ( ec )
                                 {
                                     self->connection_close_requested( ">>> Failed to send packet", reason, ec,
                                                                       lvl::error );
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
            close_on_keep_alive_timeout_.async_wait(
                strand_.wrap( [self]( const std::error_code& ec ) { self->handle_keep_alive_timeout( ec ); } ) );
        }
    }

    void mqtt_connection::handle_keep_alive_timeout( const std::error_code& ec )
    {
        if ( !ec )
        {
            auto msg = ostringstream{};
            msg << "Keep alive timeout expired after [" << ( keep_alive_ )->count( ) << "] s";
            connection_close_requested( msg.str( ), dispatch::disconnect_reason::keep_alive_timeout_expired, ec,
                                        lvl::warning );
        }
        else if ( ec == ::asio::error::operation_aborted )
        {
            if ( socket_.is_open( ) )
            {
                // Received packet and thus cancelled this timer - start next to keep us in the loop
                close_on_keep_alive_timeout( );
            }
        }
        else
        {
            assert( false );  // We should never get here
        }
    }

    void mqtt_connection::connection_close_requested( const std::string& message,
                                                      const dispatch::disconnect_reason reason,
                                                      const std::error_code& ec,
                                                      const boost::log::trivial::severity_level log_level )
    {
        auto err_info = ( ec ? " [ec:" + std::to_string( ec.value( ) ) + "|emsg:" + ec.message( ) + "]" : "" );
        BOOST_LOG_SEV( logger_, log_level )
            << "CLOSE REQUESTED (" << reason << "): " << message << err_info << " - connection will be closed";
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
