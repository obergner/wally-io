#pragma once

#include <memory>
#include <optional>
#include <string>

#include <cxxopts.hpp>

#include "io_wally/spi/authentication_service_factory.hpp"

namespace io_wally::impl
{
    /// \brief Trivial \c authentication_service that simply accepts all incoming connect requests.
    ///
    /// Use only for testing purposes, or if you really don't care, e.g because you operate in an otherwise strictly
    /// secured environment.
    class accept_all_authentication_service final : public spi::authentication_service
    {
       public:
        ~accept_all_authentication_service( ) override = default;
        /// \brief Always return \c true.
        auto authenticate( const std::string& /* client_ip */,
                           const std::optional<const std::string>& /* username  */,
                           const std::optional<const std::string> &
                           /* password */ ) -> bool override
        {
            return true;
        }
    };

    /// \brief authentication_service_factory that returns a \c accept_all_authentication_service instance.
    ///
    /// \see accept_all_authentication_service
    class accept_all_authentication_service_factory : public spi::authentication_service_factory
    {
       public:
        accept_all_authentication_service_factory( )
        {
            return;
        }

        auto operator( )( const cxxopts::ParseResult & /* config */ ) -> std::unique_ptr<spi::authentication_service>
        {
            return std::unique_ptr<spi::authentication_service>( new accept_all_authentication_service( ) );
        }
    };
}  // namespace io_wally::impl
