#include <gtest/gtest.h>
#include <net/addrs.hpp>

// =============================================================================
// Тесты для SocketAddr
// =============================================================================

TEST(SocketAddrTest, DefaultConstructors) {
    {
        net::SocketAddr sa;
        EXPECT_EQ(sa.ip(), "0.0.0.0");
        EXPECT_EQ(sa.port(), 0);
    }

    {
        net::SocketAddr sa = "192.168.1.1:8080";
        EXPECT_EQ(sa.ip(), "192.168.1.1");
        EXPECT_EQ(sa.port(), 8080);
    }

    {
        net::SocketAddr sa("192.168.1.1:8080");
        EXPECT_EQ(sa.ip(), "192.168.1.1");
        EXPECT_EQ(sa.port(), 8080);
    }

    {
        net::SocketAddr sa = "localhost:4001";
        EXPECT_EQ(sa.ip(), "127.0.0.1");
        EXPECT_EQ(sa.port(), 4001);
    }
}

TEST(SocketAddrTest, ParsePortBounds) {
    {
        net::SocketAddr sa = "10.1.0.1:0";
        EXPECT_EQ(sa.port(), 0);
    }
    {
        net::SocketAddr sa = "10.1.0.1:65535";
        EXPECT_EQ(sa.port(), 65535);
    }
}

TEST(SocketAddrTest, InvalidIpThrows) {
    EXPECT_THROW({
        [[maybe_unused]] net::SocketAddr sa = "256.256.256.256:80";
    }, std::out_of_range);

    EXPECT_THROW({
        [[maybe_unused]] net::SocketAddr sa = "10.1.0.1:65536";
    }, std::out_of_range);

    EXPECT_THROW({
        [[maybe_unused]] net::SocketAddr sa = "10.1.0:80";
    }, std::invalid_argument);

    EXPECT_THROW({
        [[maybe_unused]] net::SocketAddr sa = "10.1.0.:80";
    }, std::invalid_argument);
}

// Тест на интеграцию с оператором <=>
TEST(SocketAddrTest, Comparisons) {
    net::SocketAddr sa1 = "192.168.1.1:8080";
    net::SocketAddr sa2 = "192.168.1.1:8080";
    net::SocketAddr sa3 = "192.168.1.1:9000";

    EXPECT_EQ(sa1, sa2);
    EXPECT_NE(sa1, sa3);
    EXPECT_LT(sa1, sa3); // Порт 8080 меньше 9000
}


// =============================================================================
// Тесты для IpAddr и Взаимодействия типов (Кросс-конструкторы)
// =============================================================================

TEST(IpAddrTest, AllConstructorsAndDns) {
    // Чистый IP без порта
    net::IpAddr ip1("192.168.1.1");
    EXPECT_EQ(ip1.ip(), "192.168.1.1");

    // IP с портом (должен отсечься)
    net::IpAddr ip2("10.0.0.1:8080");
    EXPECT_EQ(ip2.ip(), "10.0.0.1");

    // DNS имя
    net::IpAddr ip3("localhost");
    EXPECT_EQ(ip3.ip(), "127.0.0.1");

    // DNS имя с портом
    net::IpAddr ip4("localhost:443");
    EXPECT_EQ(ip4.ip(), "127.0.0.1");
}

TEST(IpAddrTest, CrossConversions) {
    // Тест: Из SocketAddr в IpAddr
    net::SocketAddr saddr = "localhost:4001";
    net::IpAddr ipa = saddr;
    EXPECT_EQ(ipa.ip(), "127.0.0.1");

    // Тест: Из IpAddr в SocketAddr
    net::IpAddr ipa2 = "localhost";
    net::SocketAddr sa(ipa2, 8080);
    EXPECT_EQ(sa.ip(), "127.0.0.1");
    EXPECT_EQ(sa.port(), 8080);
}

TEST(IpAddrTest, Comparisons) {
    net::IpAddr ip1("127.0.0.1");
    net::IpAddr ip2("127.0.0.1");
    net::IpAddr ip3("192.168.1.1");

    EXPECT_EQ(ip1, ip2);
    EXPECT_NE(ip1, ip3);
    EXPECT_LT(ip1, ip3); // 127.0.0.1 меньше чем 192.168.1.1
}
