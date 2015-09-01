#pragma once

#include <functional>
#include <memory>

using namespace std;

namespace io_wally
{
    /// \brief Namespace defining interfaces to be implemented by "user" code that plugs in to the WallyIO
    ///        framework.
    namespace spi
    {
        typedef function<bool( const string& client_ip, const string& username, const string& password )>
            authentication_service;

        typedef function<unique_ptr<authentication_service>( const string& service_name )>
            authentication_service_factory;
    }  // namespace spi
}  // namespace io_wally
