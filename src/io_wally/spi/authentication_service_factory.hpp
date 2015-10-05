#pragma once

#include <functional>
#include <memory>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>

namespace io_wally
{
    using namespace std;

    /// \brief Namespace defining interfaces to be implemented by "user" code that plugs in to the WallyIO
    ///        framework.
    namespace spi
    {
        class authentication_service
        {
           public:
            virtual bool authenticate( const string& client_ip,
                                       const boost::optional<const string>& username,
                                       const boost::optional<const string>& password ) = 0;
        };

        typedef function<unique_ptr<authentication_service>( const boost::program_options::variables_map& config )>
            authentication_service_factory;
    }  // namespace spi
}  // namespace io_wally