#include <iostream>
#include <thread>
#include <chrono>

#include <net/addrs.hpp>
#include <net/tcp.hpp>
#include <net/events.hpp>

int main() {
    try {
        net::TcpListener listener = net::TcpListener::bind(net::SocketAddr("192.168.1.134:0"));
        std::cout << "Listening on: " << listener.address() << std::endl;

        net::EventLoop loop;
        loop.add_listener(listener);

        std::jthread worker([&loop]() {
            loop.run();
        });

        std::this_thread::sleep_for(std::chrono::seconds(5));
        loop.stop();
    } catch(const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}