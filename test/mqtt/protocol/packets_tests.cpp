#include "catch.hpp"

#include "mqtt/protocol/packets.hpp"

using namespace mqtt::protocol;

TEST_CASE("MQTT header flags are read", "[header]") {

    SECTION("all flags are set") {
        const uint8_t flgs = 0x0F;
        header_flags under_test(flgs);

        REQUIRE(under_test.dup());
        REQUIRE(under_test.retain());
        REQUIRE(under_test.qos() == RESERVED);
    }

    SECTION("only flags 0 and 1 are set") {
        const uint8_t flgs = 0x03;
        header_flags under_test(flgs);

        REQUIRE(!under_test.dup());
        REQUIRE(under_test.retain());
        REQUIRE(under_test.qos() == AT_LEAST_ONCE);
    }

    SECTION("only flags 1 and 2 are set") {
        const uint8_t flgs = 0x06;
        header_flags under_test(flgs);

        REQUIRE(!under_test.dup());
        REQUIRE(!under_test.retain());
        REQUIRE(under_test.qos() == RESERVED);
    }

    SECTION("only flags 2 and 3 are set") {
        const uint8_t flgs = 0x0C;
        header_flags under_test(flgs);

        REQUIRE(under_test.dup());
        REQUIRE(!under_test.retain());
        REQUIRE(under_test.qos() == EXACTLY_ONCE);
    }
}

TEST_CASE("An MQTT header is read", "[header]") {

    SECTION("no control packet type bits/no flags are set") {
        const uint8_t type_and_flags = 0x00;
        const header under_test(type_and_flags);

        REQUIRE(under_test.type() == RESERVED1);
    }

    SECTION("control packet type bit 4/no flags are set") {
        const uint8_t type_and_flags = 0x10;
        const header under_test(type_and_flags);

        REQUIRE(under_test.type() == CONNECT);
    }

    SECTION("control packet type bit 5/no flags are set") {
        const uint8_t type_and_flags = 0x20;
        const header under_test(type_and_flags);

        REQUIRE(under_test.type() == CONNACK);
    }

    SECTION("control packet type bits 4 and 5/no flags are set") {
        const uint8_t type_and_flags = 0x30;
        const header under_test(type_and_flags);

        REQUIRE(under_test.type() == PUBLISH);
    }

    SECTION("control packet type bit 6/no flags are set") {
        const uint8_t type_and_flags = 0x40;
        const header under_test(type_and_flags);

        REQUIRE(under_test.type() == PUBACK);
    }

    SECTION("control packet type bits 6 and 4/no flags are set") {
        const uint8_t type_and_flags = 0x50;
        const header under_test(type_and_flags);

        REQUIRE(under_test.type() == PUBREC);
    }

    SECTION("control packet type bits 6 and 5/no flags are set") {
        const uint8_t type_and_flags = 0x60;
        const header under_test(type_and_flags);

        REQUIRE(under_test.type() == PUBREL);
    }

    SECTION("control packet type bits 6, 5 and 4/no flags are set") {
        const uint8_t type_and_flags = 0x70;
        const header under_test(type_and_flags);

        REQUIRE(under_test.type() == PUBCOMP);
    }

    SECTION("control packet type bit 7/no flags are set") {
        const uint8_t type_and_flags = 0x80;
        const header under_test(type_and_flags);

        REQUIRE(under_test.type() == SUBSCRIBE);
    }

    SECTION("control packet type bits 7 and 4/no flags are set") {
        const uint8_t type_and_flags = 0x90;
        const header under_test(type_and_flags);

        REQUIRE(under_test.type() == SUBACK);
    }

    SECTION("control packet type bits 7 and 5/no flags are set") {
        const uint8_t type_and_flags = 0xA0;
        const header under_test(type_and_flags);

        REQUIRE(under_test.type() == UNSUBSCRIBE);
    }

    SECTION("control packet type bits 7, 5 and 4/no flags are set") {
        const uint8_t type_and_flags = 0xB0;
        const header under_test(type_and_flags);

        REQUIRE(under_test.type() == UNSUBACK);
    }

    SECTION("control packet type bits 7 and 6/no flags are set") {
        const uint8_t type_and_flags = 0xC0;
        const header under_test(type_and_flags);

        REQUIRE(under_test.type() == PINGREQ);
    }

    SECTION("control packet type bits 7, 6 and 4/no flags are set") {
        const uint8_t type_and_flags = 0xD0;
        const header under_test(type_and_flags);

        REQUIRE(under_test.type() == PINGRESP);
    }

    SECTION("control packet type bits 7, 6 and 5/no flags are set") {
        const uint8_t type_and_flags = 0xE0;
        const header under_test(type_and_flags);

        REQUIRE(under_test.type() == DISCONNECT);
    }

    SECTION("all control packet type bits/no flags are set") {
        const uint8_t type_and_flags = 0xF0;
        const header under_test(type_and_flags);

        REQUIRE(under_test.type() == RESERVED2);
    }

    SECTION("all control packet type bits/flag bit 0 are set") {
        const uint8_t type_and_flags = 0xF1;
        const header under_test(type_and_flags);

        REQUIRE(under_test.flags().retain());
    }
}
