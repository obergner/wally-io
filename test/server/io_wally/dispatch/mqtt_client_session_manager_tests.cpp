#include "catch.hpp"

#include <algorithm>

#include <asio.hpp>

#include "framework/factories.hpp"
#include "framework/mocks.hpp"

#include "io_wally/dispatch/common.hpp"
#include "io_wally/dispatch/mqtt_client_session_manager.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_packet.hpp"

using namespace io_wally::protocol;

SCENARIO( "mqtt_client_session_manager#client_published", "[dispatch]" )
{
    GIVEN( "mqtt_client_session_manager holding a connected client subscribed to a topic" )
    {
        const auto context = framework::create_context( );
        auto io_service = asio::io_service{};
        auto under_test = io_wally::dispatch::mqtt_client_session_manager{context, io_service};

        const auto subscriber_id = "test-subscriber";
        auto subscriber_ptr = std::make_shared<framework::packet_sender_mock>( subscriber_id );
        under_test.client_connected( *subscriber_ptr->client_id( ), subscriber_ptr );

        const auto topic = "/topic/mqtt_client_session_manager/test";
        const auto subscribe_packet = framework::create_subscribe_packet( {{topic, packet::QoS::AT_MOST_ONCE}} );
        under_test.client_subscribed( *subscriber_ptr->client_id( ), subscribe_packet );

        const auto publisher_id = "test-publisher";
        auto publisher_ptr = std::make_shared<framework::packet_sender_mock>( publisher_id );
        under_test.client_connected( *publisher_ptr->client_id( ), publisher_ptr );

        WHEN( "client code calls client_published() with a PUBLISH packet WITHOUT retained flag" )
        {
            auto publish_packet = framework::create_publish_packet( topic );
            publish_packet->retain( false );  // Just to be on the safe side
            under_test.client_published( publisher_id, publish_packet );

            THEN( "that PUBLISH packet should be sent to the connected client" )
            {
                const auto& sent_packets = subscriber_ptr->sent_packets( );
                const auto pos = std::find( std::begin( sent_packets ), std::end( sent_packets ), publish_packet );
                REQUIRE( pos != std::end( sent_packets ) );
            }
        }

        WHEN( "client code calls client_published() with a PUBLISH packet WITH retained flag" )
        {
            auto publish_packet = framework::create_publish_packet( topic );
            publish_packet->retain( true );
            under_test.client_published( publisher_id, publish_packet );

            THEN( "that PUBLISH packet should be sent to the connected client" )
            {
                const auto& sent_packets = subscriber_ptr->sent_packets( );
                const auto pos = std::find( std::begin( sent_packets ), std::end( sent_packets ), publish_packet );
                REQUIRE( pos != std::end( sent_packets ) );
            }
        }

        WHEN(
            "client code calls client_published() with a PUBLISH packet WITH retained flag AND THEN a new client "
            "connects and subscribes to the same topic" )
        {
            auto publish_packet = framework::create_publish_packet( topic );
            publish_packet->retain( true );
            under_test.client_published( publisher_id, publish_packet );

            const auto new_subscriber_id = "new-test-subscriber";
            auto new_subscriber_ptr = std::make_shared<framework::packet_sender_mock>( new_subscriber_id );
            under_test.client_connected( *new_subscriber_ptr->client_id( ), new_subscriber_ptr );

            const auto new_subscribe_packet =
                framework::create_subscribe_packet( {{topic, packet::QoS::AT_MOST_ONCE}} );
            under_test.client_subscribed( *new_subscriber_ptr->client_id( ), new_subscribe_packet );

            THEN( "that PUBLISH packet should be sent to the new connected client" )
            {
                const auto& sent_packets = new_subscriber_ptr->sent_packets( );
                const auto pos = std::find( std::begin( sent_packets ), std::end( sent_packets ), publish_packet );
                REQUIRE( pos != std::end( sent_packets ) );
            }
        }

        WHEN(
            "client code calls client_published() with a PUBLISH packet WITHOUT retained flag AND THEN a new client "
            "connects and subscribes to the same topic" )
        {
            auto publish_packet = framework::create_publish_packet( topic );
            publish_packet->retain( false );  // Just to be on the safe side
            under_test.client_published( publisher_id, publish_packet );

            const auto new_subscriber_id = "new-test-subscriber";
            auto new_subscriber_ptr = std::make_shared<framework::packet_sender_mock>( new_subscriber_id );
            under_test.client_connected( *new_subscriber_ptr->client_id( ), new_subscriber_ptr );

            const auto new_subscribe_packet =
                framework::create_subscribe_packet( {{topic, packet::QoS::AT_MOST_ONCE}} );
            under_test.client_subscribed( *new_subscriber_ptr->client_id( ), new_subscribe_packet );

            THEN( "that PUBLISH packet should NOT be sent to the new connected client" )
            {
                const auto& sent_packets = new_subscriber_ptr->sent_packets( );
                const auto pos = std::find( std::begin( sent_packets ), std::end( sent_packets ), publish_packet );
                REQUIRE( pos == std::end( sent_packets ) );
            }
        }
    }
}
