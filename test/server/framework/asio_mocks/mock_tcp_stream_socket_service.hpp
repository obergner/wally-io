#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <asio.hpp>

namespace framework
{
    namespace asio_mocks
    {
        class mock_tcp_stream_socket_service : public asio::detail::service_base<mock_tcp_stream_socket_service>
        {
           private:
            // The type of the platform-specific implementation.
            typedef asio::detail::reactive_socket_service<asio::ip::tcp> service_impl_type;

           public:
            /// The type of a stream socket implementation.
            typedef typename service_impl_type::implementation_type implementation_type;
            /// The native socket type.
            typedef typename service_impl_type::native_handle_type native_handle_type;

            /// Construct a new stream socket service for the specified io_service.
            explicit mock_tcp_stream_socket_service( asio::io_service& io_service )
                : mock_tcp_stream_socket_service{
                      io_service, asio::ip::tcp::endpoint{asio::ip::address_v4::from_string( "127.0.0.1" ), 12345},
                      asio::ip::tcp::endpoint{asio::ip::address_v4::from_string( "168.0.1.2" ), 34567}}
            {
            }

            /// Construct a new stream socket service for the specified io_service.
            mock_tcp_stream_socket_service( asio::io_service& io_service,
                                            const asio::ip::tcp::endpoint local_endpoint,
                                            const asio::ip::tcp::endpoint remote_endpoint )
                : asio::detail::service_base<mock_tcp_stream_socket_service>{io_service},
                  local_endpoint_{std::move( local_endpoint )},
                  remote_endpoint_{std::move( remote_endpoint )}
            {
            }

            /// Construct a new stream socket implementation.
            void construct( implementation_type& /* impl */ )
            {
            }

            /// Move-construct a new stream socket implementation.
            void move_construct( implementation_type& /*  impl */, implementation_type& /*  other_impl */ )
            {
            }

            /// Move-assign from another stream socket implementation.
            void move_assign( implementation_type& /*  impl */,
                              mock_tcp_stream_socket_service& /*  other_service */,
                              implementation_type& /* other_impl */ )
            {
            }

            /// Move-construct a new stream socket implementation from another protocol
            /// type.
            template <typename Protocol1>
            void converting_move_construct(
                implementation_type& /* impl */,
                typename asio::stream_socket_service<Protocol1>::implementation_type& /* other_impl */,
                typename asio::enable_if<asio::is_convertible<Protocol1, asio::ip::tcp>::value>::type* = 0 )
            {
            }

            /// Destroy a stream socket implementation.
            void destroy( implementation_type& /* impl */ )
            {
            }

            /// Open a stream socket.
            asio::error_code open( implementation_type&, const asio::ip::tcp&, asio::error_code& ec )
            {
                return ec;
            }

            /// Assign an existing native socket to a stream socket.
            asio::error_code assign( implementation_type&,
                                     const asio::ip::tcp&,
                                     const native_handle_type&,
                                     asio::error_code& ec )
            {
                return ec;
            }

            /// Determine whether the socket is open.
            bool is_open( const implementation_type& ) const
            {
                return true;
            }

            /// Close a stream socket implementation.
            asio::error_code close( implementation_type&, asio::error_code& ec )
            {
                return ec;
            }

            /// Cancel all asynchronous operations associated with the socket.
            asio::error_code cancel( implementation_type&, asio::error_code& ec )
            {
                return ec;
            }

            /// Determine whether the socket is at the out-of-band data mark.
            bool at_mark( const implementation_type&, asio::error_code& ) const
            {
                return false;
            }

            /// Determine the number of bytes available for reading.
            std::size_t available( const implementation_type&, asio::error_code& ) const
            {
                return 1;
            }

            /// Bind the stream socket to the specified local endpoint.
            asio::error_code bind( implementation_type&, const asio::ip::tcp::endpoint&, asio::error_code& ec )
            {
                return ec;
            }

            /// Connect the stream socket to the specified endpoint.
            asio::error_code connect( implementation_type&, const asio::ip::tcp::endpoint&, asio::error_code& ec )
            {
                return ec;
            }

            /// Set a socket option.
            template <typename SettableSocketOption>
            asio::error_code set_option( implementation_type&, const SettableSocketOption&, asio::error_code& ec )
            {
                return ec;
            }

            /// Get a socket option.
            template <typename GettableSocketOption>
            asio::error_code get_option( const implementation_type&, GettableSocketOption&, asio::error_code& ec ) const
            {
                return ec;
            }

            /// Perform an IO control command on the socket.
            template <typename IoControlCommand>
            asio::error_code io_control( implementation_type&, IoControlCommand&, asio::error_code& ec )
            {
                return ec;
            }

            /// Gets the non-blocking mode of the socket.
            bool non_blocking( const implementation_type& ) const
            {
                return true;
            }

            /// Sets the non-blocking mode of the socket.
            asio::error_code non_blocking( implementation_type&, bool, asio::error_code& ec )
            {
                return ec;
            }

            /// Gets the non-blocking mode of the native socket implementation.
            bool native_non_blocking( const implementation_type& ) const
            {
                return true;
            }

            /// Sets the non-blocking mode of the native socket implementation.
            asio::error_code native_non_blocking( implementation_type&, bool, asio::error_code& ec )
            {
                return ec;
            }

            /// Get the local endpoint.
            asio::ip::tcp::endpoint local_endpoint( const implementation_type&, asio::error_code& ) const
            {
                return local_endpoint_;
            }

            /// Get the remote endpoint.
            asio::ip::tcp::endpoint remote_endpoint( const implementation_type&, asio::error_code& ) const
            {
                return remote_endpoint_;
            }

            /// Disable sends or receives on the socket.
            asio::error_code shutdown( implementation_type&, asio::socket_base::shutdown_type, asio::error_code& ec )
            {
                return ec;
            }

            /// Send the given data to the peer.
            template <typename ConstBufferSequence>
            std::size_t send( implementation_type&,
                              const ConstBufferSequence&,
                              asio::socket_base::message_flags,
                              asio::error_code& )
            {
                return 1;
            }

            /// Start an asynchronous send.
            template <typename ConstBufferSequence, typename WriteHandler>
            void async_send( implementation_type&,
                             const ConstBufferSequence&,
                             asio::socket_base::message_flags,
                             ASIO_MOVE_ARG( WriteHandler ) )
            {
            }

            /// Receive some data from the peer.
            template <typename MutableBufferSequence>
            std::size_t receive( implementation_type&,
                                 const MutableBufferSequence&,
                                 asio::socket_base::message_flags,
                                 asio::error_code& )
            {
                return 1;
            }

            /// Start an asynchronous receive.
            template <typename MutableBufferSequence, typename ReadHandler>
            void async_receive( implementation_type&,
                                const MutableBufferSequence&,
                                asio::socket_base::message_flags,
                                ASIO_MOVE_ARG( ReadHandler ) )
            {
            }

           private:
            void shutdown_service( ) override
            {
            }

           private:
            const asio::ip::tcp::endpoint local_endpoint_;
            const asio::ip::tcp::endpoint remote_endpoint_;
        };
    }  // namespace asio_mocks
}  // namespace framework
