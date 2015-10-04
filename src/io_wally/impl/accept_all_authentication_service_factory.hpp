#pragma once

#include <memory>
#include <string>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include "io_wally/spi/authentication_service_factory.hpp"

namespace io_wally
{
    using namespace std;
    namespace options = boost::program_options;

    namespace impl
    {
        /// \brief Trivial \c authentication_service that simply accepts all incoming connect requests.
        ///
        /// Use only for testing purposes, or if you really don't care, e.g because you operate in an otherwise strictly
        /// secured environment.
        class accept_all_authentication_service : public spi::authentication_service
        {
           public:
            /// \brief Always return \c true.
            bool authenticate( const string& /* client_ip */,
                               const boost::optional<const string>& /* username  */,
                               const boost::optional<const string>& /* password */ ) override
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

            unique_ptr<spi::authentication_service> operator( )( const options::variables_map& /* config */ )
            {
                return unique_ptr<spi::authentication_service>( new accept_all_authentication_service( ) );
            }
        };
    }  // namespace spi
}  // namespace io_wally
