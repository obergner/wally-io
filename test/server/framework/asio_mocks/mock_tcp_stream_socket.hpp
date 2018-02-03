#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <asio.hpp>

#include "mock_tcp_stream_socket_service.hpp"

namespace framework
{
    namespace asio_mocks
    {
        using mock_tcp_stream_socket = asio::basic_stream_socket<asio::ip::tcp, mock_tcp_stream_socket_service>;
    }  // namespace asio_mocks
}  // namespace framework
