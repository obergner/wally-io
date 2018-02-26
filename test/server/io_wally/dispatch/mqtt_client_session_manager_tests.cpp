#include "catch.hpp"

#include <algorithm>
#include <string>

#include <asio.hpp>

#include "framework/factories.hpp"
#include "framework/mocks.hpp"

#include "io_wally/dispatch/common.hpp"
#include "io_wally/dispatch/mqtt_client_session_manager.hpp"
#include "io_wally/protocol/common.hpp"
#include "io_wally/protocol/publish_packet.hpp"

using namespace std::string_literals;
using namespace io_wally::protocol;

SCENARIO( "mqtt_client_session_manager::session_store#insert", "[dispatch]" )
{
    GIVEN( "an initially empty instance of mqtt_client_session_manager::session_store" )
    {
        const auto context = framework::create_context( );
        auto io_service = asio::io_service{};
        auto parent = io_wally::dispatch::mqtt_client_session_manager{context, io_service};

        auto under_test = io_wally::dispatch::mqtt_client_session_manager::session_store{parent};

        WHEN( "client code calls under_test.insert(...)" )
        {
            const auto client_id = "test-client"s;
            const auto connect = framework::create_connect_packet( client_id );
            auto client_connection_ptr = std::make_shared<framework::packet_sender_mock>( client_id );

            const auto inserted = under_test.insert( connect, client_connection_ptr );

            THEN( "that client code should see true being returned" )
            {
                REQUIRE( inserted );
            }
        }

        WHEN( "client code calls under_test.insert(...) twice with the same client_id" )
        {
            const auto client_id = "test-client"s;

            const auto connect1 = framework::create_connect_packet( client_id );
            auto client1_connection_ptr = std::make_shared<framework::packet_sender_mock>( client_id );
            under_test.insert( connect1, client1_connection_ptr );

            const auto connect2 = framework::create_connect_packet( client_id );
            auto client2_connection_ptr = std::make_shared<framework::packet_sender_mock>( client_id );
            const auto inserted2 = under_test.insert( connect2, client2_connection_ptr );

            THEN( "that client code should see false being returned" )
            {
                REQUIRE( !inserted2 );
            }
        }

        WHEN( "client code calls under_test.insert(client_id, connection_ptr)" )
        {
            const auto client_id = "test-client"s;
            const auto connect = framework::create_connect_packet( client_id );
            auto client_connection_ptr = std::make_shared<framework::packet_sender_mock>( client_id );

            under_test.insert( connect, client_connection_ptr );

            THEN( "calling under_test[client_id] should return newly created session" )
            {
                const auto stored_session = under_test[client_id];
                CHECK( stored_session );
                REQUIRE( stored_session->client_id( ) == client_id );
            }
        }
    }
}

SCENARIO( "mqtt_client_session_manager::session_store#size", "[dispatch]" )
{
    GIVEN( "an initially empty instance of mqtt_client_session_manager::session_store" )
    {
        const auto context = framework::create_context( );
        auto io_service = asio::io_service{};
        auto parent = io_wally::dispatch::mqtt_client_session_manager{context, io_service};

        auto under_test = io_wally::dispatch::mqtt_client_session_manager::session_store{parent};

        const auto client_id = "test-client"s;

        WHEN( "client code calls under_test.insert(...) x times with connections having different client_ids" )
        {
            static constexpr const std::size_t x = 10;
            const auto ids = std::array<const int, x>{{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}};
            for ( const auto id : ids )
            {
                const auto connect = framework::create_connect_packet( client_id + std::to_string( id ) );
                auto client_connection_ptr =
                    std::make_shared<framework::packet_sender_mock>( client_id + std::to_string( id ) );
                under_test.insert( connect, client_connection_ptr );
            }

            THEN( "that client code should see x being returned from under_test.size()" )
            {
                REQUIRE( under_test.size( ) == x );
            }
        }
    }
}

SCENARIO( "mqtt_client_session_manager::session_store#remove", "[dispatch]" )
{
    GIVEN( "an initially empty instance of mqtt_client_session_manager::session_store" )
    {
        const auto context = framework::create_context( );
        auto io_service = asio::io_service{};
        auto parent = io_wally::dispatch::mqtt_client_session_manager{context, io_service};

        auto under_test = io_wally::dispatch::mqtt_client_session_manager::session_store{parent};

        const auto client_id = "test-client"s;

        WHEN( "client code calls under_test.insert(...)" )
        {
            const auto connect = framework::create_connect_packet( client_id );
            auto client_connection_ptr = std::make_shared<framework::packet_sender_mock>( client_id );

            const auto inserted = under_test.insert( connect, client_connection_ptr );

            THEN( "that client code should see true being returned" )
            {
                REQUIRE( inserted );
            }
        }

        AND_WHEN( "client code calls under_test.remove(...) with the same client_id" )
        {
            under_test.remove( client_id );

            THEN( "that client code should see a nullptr being returned from under_test[...]" )
            {
                REQUIRE( !under_test[client_id] );
            }
        }
    }
}

SCENARIO( "mqtt_client_session_manager::session_store#operator[]", "[dispatch]" )
{
    GIVEN( "an initially empty instance of mqtt_client_session_manager::session_store" )
    {
        const auto context = framework::create_context( );
        auto io_service = asio::io_service{};
        auto parent = io_wally::dispatch::mqtt_client_session_manager{context, io_service};

        auto under_test = io_wally::dispatch::mqtt_client_session_manager::session_store{parent};

        WHEN( "client code calls under_test['some-client-id']" )
        {
            const auto client_id = "some-client-id"s;
            const auto non_existing_session = under_test[client_id];

            THEN( "that client code should see a nullptr being returned" )
            {
                REQUIRE( !non_existing_session );
            }
        }

        WHEN( "client code calls under_test.insert(client_id, connection_ptr)" )
        {
            const auto client_id = "test-client"s;
            const auto connect = framework::create_connect_packet( client_id );
            auto client_connection_ptr = std::make_shared<framework::packet_sender_mock>( client_id );

            under_test.insert( connect, client_connection_ptr );

            THEN( "calling under_test[client_id] should return newly created session" )
            {
                const auto stored_session = under_test[client_id];
                CHECK( stored_session );
                REQUIRE( stored_session->client_id( ) == client_id );
            }
        }
    }
}

SCENARIO( "mqtt_client_session_manager::session_store#clear", "[dispatch]" )
{
    GIVEN( "an initially empty instance of mqtt_client_session_manager::session_store" )
    {
        const auto context = framework::create_context( );
        auto io_service = asio::io_service{};
        auto parent = io_wally::dispatch::mqtt_client_session_manager{context, io_service};

        auto under_test = io_wally::dispatch::mqtt_client_session_manager::session_store{parent};

        const auto client_id = "test-client"s;

        WHEN( "client code calls under_test.insert(...) x times with connections having different client_ids" )
        {
            static constexpr const std::size_t x = 10;
            const auto ids = std::array<const int, x>{{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}};
            for ( const auto id : ids )
            {
                const auto connect = framework::create_connect_packet( client_id + std::to_string( id ) );
                auto client_connection_ptr =
                    std::make_shared<framework::packet_sender_mock>( client_id + std::to_string( id ) );
                under_test.insert( connect, client_connection_ptr );
            }
        }

        AND_WHEN( "client code then calls under_test.clear()" )
        {
            under_test.clear( );

            THEN( "calling under_test.size() should return 0" )
            {
                REQUIRE( under_test.size( ) == 0 );
            }
        }
    }
}

SCENARIO( "mqtt_client_session_manager#client_published", "[dispatch]" )
{
    GIVEN( "mqtt_client_session_manager holding a connected client subscribed to a topic" )
    {
        const auto context = framework::create_context( );
        auto io_service = asio::io_service{};
        auto under_test = io_wally::dispatch::mqtt_client_session_manager{context, io_service};

        const auto subscriber_id = "test-subscriber"s;
        const auto subscriber_connect = framework::create_connect_packet( subscriber_id );
        auto subscriber_ptr = std::make_shared<framework::packet_sender_mock>( subscriber_id );
        under_test.client_connected( subscriber_connect, subscriber_ptr );

        const auto topic = "/topic/mqtt_client_session_manager/test"s;
        const auto subscribe_packet = framework::create_subscribe_packet( {{topic, packet::QoS::AT_MOST_ONCE}} );
        under_test.client_subscribed( *subscriber_ptr->client_id( ), subscribe_packet );

        const auto publisher_id = "test-publisher"s;
        const auto publisher_connect = framework::create_connect_packet( publisher_id );
        auto publisher_ptr = std::make_shared<framework::packet_sender_mock>( publisher_id );
        under_test.client_connected( publisher_connect, publisher_ptr );

        WHEN( "client code calls client_published() with a PUBLISH packet WITHOUT retained flag" )
        {
            auto publish_packet = framework::create_publish_packet( topic, false );
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
            auto publish_packet = framework::create_publish_packet( topic, true );
            under_test.client_published( publisher_id, publish_packet );

            THEN( "that PUBLISH packet should be sent to the connected client" )
            {
                const auto& sent_packets = subscriber_ptr->sent_packets( );
                const auto pos = std::find( std::begin( sent_packets ), std::end( sent_packets ), publish_packet );
                REQUIRE( pos != std::end( sent_packets ) );
            }
        }

        WHEN( "client code calls client_published() with a PUBLISH packet of size 0 WITH retained flag" )
        {
            auto publish_packet = framework::create_publish_packet( topic, true, {} );
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
            auto publish_packet = framework::create_publish_packet( topic, true );
            under_test.client_published( publisher_id, publish_packet );

            const auto new_subscriber_id = "new-test-subscriber"s;
            const auto new_subscriber_connect = framework::create_connect_packet( new_subscriber_id );
            auto new_subscriber_ptr = std::make_shared<framework::packet_sender_mock>( new_subscriber_id );
            under_test.client_connected( new_subscriber_connect, new_subscriber_ptr );

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
            auto publish_packet = framework::create_publish_packet( topic, false );
            under_test.client_published( publisher_id, publish_packet );

            const auto new_subscriber_id = "new-test-subscriber"s;
            const auto new_subscriber_connect = framework::create_connect_packet( new_subscriber_id );
            auto new_subscriber_ptr = std::make_shared<framework::packet_sender_mock>( new_subscriber_id );
            under_test.client_connected( new_subscriber_connect, new_subscriber_ptr );

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

        WHEN(
            "client code first calls client_published() with a regular PUBLISH packet WITH retained flag, THEN with a "
            "PUBLISH packet of size 0 WITH retained flag AND THEN a new client connects and subscribes to the same "
            "topic" )
        {
            auto regular_publish_packet = framework::create_publish_packet( topic, true );
            under_test.client_published( publisher_id, regular_publish_packet );

            auto delete_publish_packet = framework::create_publish_packet( topic, true, {} );
            under_test.client_published( publisher_id, delete_publish_packet );

            const auto new_subscriber_id = "new-test-subscriber"s;
            const auto new_subscriber_connect = framework::create_connect_packet( new_subscriber_id );
            auto new_subscriber_ptr = std::make_shared<framework::packet_sender_mock>( new_subscriber_id );
            under_test.client_connected( new_subscriber_connect, new_subscriber_ptr );

            const auto new_subscribe_packet =
                framework::create_subscribe_packet( {{topic, packet::QoS::AT_MOST_ONCE}} );
            under_test.client_subscribed( *new_subscriber_ptr->client_id( ), new_subscribe_packet );

            THEN( "NO PUBLISH packet should be sent to the new connected client" )
            {
                const auto& sent_packets = new_subscriber_ptr->sent_packets( );
                const auto regular_pos =
                    std::find( std::begin( sent_packets ), std::end( sent_packets ), regular_publish_packet );
                const auto delete_pos =
                    std::find( std::begin( sent_packets ), std::end( sent_packets ), delete_publish_packet );
                CHECK( regular_pos == std::end( sent_packets ) );
                REQUIRE( delete_pos == std::end( sent_packets ) );
            }
        }
    }
}

SCENARIO( "mqtt_client_session_manager#client_disconnected_ungracefully", "[dispatch]" )
{
    GIVEN( "mqtt_client_session_manager holding one connected client" )
    {
        const auto context = framework::create_context( );
        auto io_service = asio::io_service{};
        auto under_test = io_wally::dispatch::mqtt_client_session_manager{context, io_service};

        const auto client_id = "test-client"s;
        const auto connect = framework::create_connect_packet( client_id );
        auto client_ptr = std::make_shared<framework::packet_sender_mock>( client_id );
        under_test.client_connected( connect, client_ptr );

        WHEN( "client code calls under_test.client_disconnected_ungracefully(...)" )
        {
            under_test.client_disconnected_ungracefully( client_id,
                                                         io_wally::dispatch::disconnect_reason::protocol_violation );

            THEN( "under_test.connected_clients_count() should return 0" )
            {
                REQUIRE( under_test.connected_clients_count( ) == 0 );
            }
        }
    }

    GIVEN(
        "mqtt_client_session_manager holding a connected client with an unretained LWT message and a connected client "
        "subscribed to LWT topic" )
    {
        const auto context = framework::create_context( );
        auto io_service = asio::io_service{};
        auto under_test = io_wally::dispatch::mqtt_client_session_manager{context, io_service};

        const auto lwt_topic = "/topic/mqtt_client_session_manager/unretained-lwt/test"s;

        const auto lwt_client_id = "unretained-lwt-publisher"s;
        const auto lwt_message = "LWT"s;
        const auto lwt_client_connect = framework::create_connect_packet_with_lwt(
            lwt_client_id, false, lwt_topic, lwt_message, io_wally::protocol::packet::QoS::AT_MOST_ONCE, false );
        auto lwt_client_ptr = std::make_shared<framework::packet_sender_mock>( lwt_client_id );
        under_test.client_connected( lwt_client_connect, lwt_client_ptr );

        const auto lwt_subscriber_id = "unretained-lwt-subscriber"s;
        const auto lwt_subscriber_connect = framework::create_connect_packet( lwt_subscriber_id );
        auto lwt_subscriber_ptr = std::make_shared<framework::packet_sender_mock>( lwt_subscriber_id );
        under_test.client_connected( lwt_subscriber_connect, lwt_subscriber_ptr );

        const auto subscribe_packet = framework::create_subscribe_packet( {{lwt_topic, packet::QoS::AT_MOST_ONCE}} );
        under_test.client_subscribed( *lwt_client_ptr->client_id( ), subscribe_packet );
        under_test.client_subscribed( *lwt_subscriber_ptr->client_id( ), subscribe_packet );

        WHEN( "client code calls client_disconnected_ungracefully() for the connected client with LWT message" )
        {
            under_test.client_disconnected_ungracefully(
                lwt_client_id, io_wally::dispatch::disconnect_reason::network_or_server_failure );

            THEN( "LWT message should be sent to subscribed client" )
            {
                const auto& sent_packets = lwt_subscriber_ptr->sent_packets( );
                const auto lwt_message_vec = std::vector<uint8_t>{lwt_message.begin( ), lwt_message.end( )};
                const auto pos = std::find_if(
                    std::begin( sent_packets ), std::end( sent_packets ),
                    [&lwt_message_vec]( const std::shared_ptr<const io_wally::protocol::mqtt_packet> packet ) -> bool {
                        return ( packet->type( ) == io_wally::protocol::packet::Type::PUBLISH ) &&
                               ( std::dynamic_pointer_cast<const io_wally::protocol::publish>( packet )
                                     ->application_message( ) == lwt_message_vec );
                    } );
                REQUIRE( pos != std::end( sent_packets ) );
            }

            AND_THEN( "LWT message should NOT be sent to the disconnected client" )
            {
                const auto& sent_packets = lwt_client_ptr->sent_packets( );
                const auto lwt_message_vec = std::vector<uint8_t>{lwt_message.begin( ), lwt_message.end( )};
                const auto pos = std::find_if(
                    std::begin( sent_packets ), std::end( sent_packets ),
                    [&lwt_message_vec]( const std::shared_ptr<const io_wally::protocol::mqtt_packet> packet ) -> bool {
                        return ( packet->type( ) == io_wally::protocol::packet::Type::PUBLISH ) &&
                               ( std::dynamic_pointer_cast<const io_wally::protocol::publish>( packet )
                                     ->application_message( ) == lwt_message_vec );
                    } );
                REQUIRE( pos == std::end( sent_packets ) );
            }
        }
    }
}
