#pragma once

#include "io_wally/spi/authentication_service_factory.hpp"

using namespace std;
using namespace io_wally::spi;

namespace io_wally
{
    namespace impl
    {
        /// \brief Trivial \c authentication_service that simply accepts all incoming connect requests.
        ///
        /// Use only for testing purposes, or if you really don't care, e.g because you operate in an otherwise strictly
        /// secured environment.
        class accept_all_authentication_service : public authentication_service
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
        class accept_all_authentication_service_factory : public authentication_service_factory
        {
           public:
            accept_all_authentication_service_factory( )
            {
                return;
            }

            unique_ptr<authentication_service> operator( )( const options::variables_map& /* config */ )
            {
                return unique_ptr<authentication_service>( new accept_all_authentication_service( ) );
            }
        };
    }  // namespace spi
}  // namespace io_wally
