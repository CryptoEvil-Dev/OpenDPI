#define DEBUG

#include <iostream>
#include <sock.hpp>

int main(int argc, char **argv) {

    net::SocketAddr sa = "192.168.1.54:8082";
    std::cout << sa.ip() << ":" << sa.port() << std::endl;

    return 0;
}