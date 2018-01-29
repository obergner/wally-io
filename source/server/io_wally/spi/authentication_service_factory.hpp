#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include <cxxopts.hpp>

namespace io_wally
{
    /// \brief Namespace defining interfaces to be implemented by "user" code that plugs in to the WallyIO
    ///        framework.
    namespace spi
    {
        class authentication_service
        {
           public:
            virtual bool authenticate( const std::string& client_ip,
                                       const std::optional<const std::string>& username,
                                       const std::optional<const std::string>& password ) = 0;
        };

        typedef std::function<std::unique_ptr<authentication_service>( const cxxopts::ParseResult& config )>
            authentication_service_factory;
    }  // namespace spi
}  // namespace io_wally
