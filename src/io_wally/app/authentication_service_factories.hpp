#pragma once

#include "io_wally/spi/authentication_service_factory.hpp"

#include "io_wally/impl/accept_all_authentication_service_factory.hpp"

namespace io_wally
{
    namespace app
    {
        /// \brief Registry for named \c authentication_service_factory instances
        ///
        /// Implements a singleton map-like class that allows for retrieving references to know \c
        /// authentication_service_factory instances by name.
        class authentication_service_factories final
        {
           public:
            /// \brief Singleton accessor.
            ///
            static authentication_service_factories& instance( )
            {
                static authentication_service_factories instance;

                return instance;
            }

           public:
            /// \brief Name of \c accept_all_authentication_service_factory.
            static const std::string ACCEPT_ALL;

           public:
            /// \brief Retrieve \c authentication_service_factory reference by name.
            ///
            const spi::authentication_service_factory& operator[]( const std::string& name ) const;

           private:
            authentication_service_factories( ) = default;

            authentication_service_factories( authentication_service_factories const& ) = delete;

            void operator=( authentication_service_factories const& ) = delete;

           private:
            const std::map<std::string, spi::authentication_service_factory> factories_by_name_{
                {ACCEPT_ALL, impl::accept_all_authentication_service_factory{}}};

        };  // class authentication_service_factories
    }
}
