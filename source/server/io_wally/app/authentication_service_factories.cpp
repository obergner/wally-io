#include "io_wally/app/authentication_service_factories.hpp"

#include "io_wally/spi/authentication_service_factory.hpp"

namespace io_wally
{
    using namespace std;

    namespace app
    {
        const string authentication_service_factories::ACCEPT_ALL = "accept_all";

        auto authentication_service_factories::operator[]( const string& name ) const
            -> const spi::authentication_service_factory&
        {
            return factories_by_name_.at( name );
        }
    }  // namespace app
}  // namespace io_wally
