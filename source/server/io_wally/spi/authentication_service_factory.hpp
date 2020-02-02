#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include <cxxopts.hpp>

namespace io_wally::spi
{
    class authentication_service
    {
       public:
        virtual ~authentication_service( ) = default;
        ;

        virtual auto authenticate( const std::string& client_ip,
                                   const std::optional<const std::string>& username,
                                   const std::optional<const std::string>& password ) -> bool = 0;
    };

    using authentication_service_factory =
        std::function<std::unique_ptr<authentication_service>( const cxxopts::ParseResult& )>;
}  // namespace io_wally::spi
