#include <gtest/gtest.h>
#include <net/tcp.hpp>
#include <thread>
#include <string>



// [new] Update tests
TEST(TcpListener, Construct) {
    net::SocketAddr sa("127.0.0.1:0");
    auto listener = net::TcpListener::bind(sa);

    EXPECT_NE(listener.address(), sa);
}

TEST(TcpListener, MultiListeners) {
    net::SocketAddr sas[5] = {
        "127.0.0.1:0",
        "127.0.0.1:0",
        "127.0.0.1:0",
        "127.0.0.1:0",
        "127.0.0.1:0",
    };
    net::TcpListener listeners[5] = {
        net::TcpListener::bind(sas[0]),
        net::TcpListener::bind(sas[1]),
        net::TcpListener::bind(sas[2]),
        net::TcpListener::bind(sas[3]),
        net::TcpListener::bind(sas[4])
    };

    for(int i = 0; i < 5; ++i) {
        EXPECT_NE(listeners[i].address(), sas[i]);
    }
}

TEST(TcpListener, MoveConstructor) {
    // Create on stack.
    net::SocketAddr vsa("127.0.0.1:0");
    net::TcpListener olistener = net::TcpListener::bind(vsa);
    net::SocketAddr rsa = olistener.address();

    // Move to heap.
    net::TcpListener* heap_listener = new net::TcpListener(std::move(olistener));
    EXPECT_EQ(heap_listener->address(), rsa);

    // Return in stack.
    net::TcpListener listener = std::move(*heap_listener);
    EXPECT_EQ(listener.address(), rsa);

    // Verify that addresses is different.
    EXPECT_NE(rsa, vsa);
}



// Flaky тест, может провалится, если на сокет шли данные из-за TIME_WAIT.
// Несмотря на SO_REUSEADDR, может потребоваться 60 секунд на cooldown порта.
TEST(TcpListener, Reopen) {
    net::TcpListener listener = net::TcpListener::bind(net::SocketAddr("127.0.0.1:0"));
    net::SocketAddr sa = listener.address();

    listener.close();
    listener = net::TcpListener::bind(sa);
    EXPECT_EQ(listener.address(), sa);
}

TEST(TcpListener, AddressPersistsAfterClose) {
    net::TcpListener listener = net::TcpListener::bind(net::SocketAddr("127.0.0.1:0"));
    net::SocketAddr sa = listener.address();
    listener.close();
    
    EXPECT_EQ(listener.address(), sa);
}

TEST(TcpStream, ReadWrite) {
    net::TcpListener mock_server = net::TcpListener::bind(net::SocketAddr("localhost:0"));

    std::jthread server_th([&mock_server]() {
        net::TcpStream client = mock_server.accept();
        std::string buffer;
        buffer.resize(1500);
        net::IoResult readed = client.read(buffer);
        if(readed.ok()) {
            buffer.resize(readed.bytes);
            EXPECT_EQ(buffer, "Server Hello");
        } else FAIL();

        buffer = "Client Hello";
        net::IoResult written = client.write(buffer);
        EXPECT_TRUE(written.ok());
        // mock_server.close();
    });

    auto conn = net::TcpStream::connect(mock_server.address());
    std::string buffer = "Server Hello";
    net::IoResult written = conn.write(buffer);
    EXPECT_TRUE(written.ok());

    buffer.resize(1500);
    net::IoResult readed = conn.read(buffer);
    if(readed.ok()) {
        buffer.resize(readed.bytes);
        EXPECT_EQ(buffer, "Client Hello");
    } else FAIL();
}

TEST(TcpStream, ReadClosedSocket) {
    net::TcpListener listener = net::TcpListener::bind(net::SocketAddr("localhost:0"));

    std::jthread server_th([&listener]() {
        auto conn = listener.accept();
        std::string buffer;
        buffer.resize(100);
        net::IoResult result = conn.read(buffer);
        EXPECT_TRUE(result.closed());
    });

    auto conn = net::TcpStream::connect(listener.address());
    conn.close();
}

TEST(TcpStream, ResponseAfterMove) {
    net::TcpListener listener = net::TcpListener::bind(net::SocketAddr("localhost:0"));

    std::jthread th([&listener]() {
        net::TcpStream old_conn = listener.accept();
        net::TcpStream* conn = new net::TcpStream(std::move(old_conn));

        std::string buffer;
        buffer.resize(1500);
        net::IoResult res = conn->read(buffer);
        if(res.ok()) {
            buffer.resize(res.bytes);
            EXPECT_EQ(buffer, "Server Hello");
        } else FAIL();

        net::TcpStream new_conn = net::TcpStream(std::move(*conn));
        buffer = "Client Hello";
        res = new_conn.write(buffer);
        EXPECT_TRUE(res.ok());
    });

    auto old_conn = net::TcpStream::connect(listener.address());
    net::TcpStream* conn = new net::TcpStream(std::move(old_conn));
    std::string buffer = "Server Hello";
    net::IoResult res = conn->write(buffer);
    if(!res.ok()) FAIL();
    buffer.resize(1500);
    
    net::TcpStream new_conn = net::TcpStream(std::move(*conn));
    res = new_conn.read(buffer);
    if(!res.ok()) FAIL();
    buffer.resize(res.bytes);
    EXPECT_EQ(buffer, "Client Hello");
}