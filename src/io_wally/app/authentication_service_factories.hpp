#pragma once

#include <io_wally/impl/accept_all_authentication_service_factory.hpp>

namespace io_wally
{
    namespace app
    {
        /// \brief Registry for named \c authentication_service_factory instances
        ///
        /// Implements a singleton map-like class that allows for retrieving references to know \c
        /// authentication_service_factory instances by name.
        class authentication_service_factories
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
            static const string ACCEPT_ALL;

           public:
            /// \brief Retrieve \c authentication_service_factory reference by name.
            ///
            const io_wally::spi::authentication_service_factory& operator[]( const string& name ) const;

           private:
            authentication_service_factories( )
            {
                return;
            }

            authentication_service_factories( authentication_service_factories const& ) = delete;

            void operator=( authentication_service_factories const& ) = delete;

           private:
            const map<string, io_wally::spi::authentication_service_factory> factories_by_name_{
                {ACCEPT_ALL, io_wally::impl::accept_all_authentication_service_factory{}}};

        };  // class authentication_service_factories
    }
}
