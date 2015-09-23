#pragma once

#include <cstdint>
#include <sstream>

#include <boost/program_options.hpp>

#include "io_wally/spi/authentication_service_factory.hpp"

using namespace std;
namespace options = boost::program_options;

namespace io_wally
{
    struct context
    {
       public:
        context( const options::variables_map options,
                 unique_ptr<io_wally::spi::authentication_service_factory> authentication_service_factory )
            : options_( move( options ) ), authentication_service_factory_( move( authentication_service_factory ) )
        {
            return;
        }

        const options::variables_map& options( ) const
        {
            return options_;
        }

        const io_wally::spi::authentication_service_factory& authentication_service_factory( ) const
        {
            return *authentication_service_factory_;
        }

       private:
        const options::variables_map options_;
        const unique_ptr<io_wally::spi::authentication_service_factory> authentication_service_factory_;
    };
}
