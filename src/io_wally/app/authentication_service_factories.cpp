#include <io_wally/app/authentication_service_factories.hpp>

namespace io_wally
{
    namespace app
    {
        const string authentication_service_factories::ACCEPT_ALL = "accept_all";

        const io_wally::spi::authentication_service_factory& authentication_service_factories::operator[](
            const string& name ) const
        {
            return factories_by_name_.at( name );
        }
    }
}
