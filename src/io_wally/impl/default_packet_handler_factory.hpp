#include "io_wally/spi/mqtt_packet_handler_factory.hpp"

using namespace io_wally::spi;

namespace io_wally
{
    namespace impl
    {
        class default_authentication_handler : public mqtt_authentication_handler
        {
           public:
            const struct connack operator( )( const struct connect& /* connect */ ) override
            {
                const struct connack ack( false, connect_return_code::CONNECTION_ACCEPTED );

                return std::move( ack );
            }
        };  // default_authentication_handler

        class default_packet_handler : public mqtt_packet_handler
        {
           public:
            std::unique_ptr<const struct mqtt_ack> operator( )( const struct mqtt_packet& /* packet */ ) override
            {
                std::unique_ptr<const struct mqtt_ack> result(
                    new connack( false, connect_return_code::CONNECTION_ACCEPTED ) );

                return std::move( result );
            }
        };  // default_packet_handler

        class default_packet_handler_factory : public mqtt_packet_handler_factory
        {
           public:
            default_packet_handler_factory( )
            {
                return;
            }

            std::unique_ptr<mqtt_authentication_handler> create_authentication_handler(
                const mqtt_session& /* session */ ) override
            {
                std::unique_ptr<mqtt_authentication_handler> result( new default_authentication_handler( ) );

                return std::move( result );
            }

            std::unique_ptr<mqtt_packet_handler> create_packet_handler( const mqtt_session_id& /* session */ ) override
            {
                std::unique_ptr<mqtt_packet_handler> result( new default_packet_handler( ) );

                return std::move( result );
            }
        };  // default_packet_handler_factory
    }       // namespace impl
}  // namespace io_wally
