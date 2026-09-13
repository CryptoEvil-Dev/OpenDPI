#pragma once

#include <arpa/inet.h>
#include <string_view>
#include <string>

namespace net {

class SocketAddr {
public:
    SocketAddr() : data({0}) {};
    SocketAddr(const SocketAddr&) = default;
    SocketAddr(SocketAddr&&) = default;


    SocketAddr(int a, int b, int c, int d, int port) : data(this->_parse_ints(a,b,c,d,port)) {};
    SocketAddr(std::string_view sv) : data(this->_parse_sv(sv)) {};
    SocketAddr(const char *address) : data(this->_parse_sv(address)) {};
    SocketAddr(struct sockaddr_in *addr) : data(*addr) {};

    SocketAddr& operator = (const SocketAddr&) = delete;
    SocketAddr& operator = (SocketAddr&& other) {
        if(this != &other) {
            this->data = std::move(other.data);
        }
        return *this;
    }

    SocketAddr& operator = (std::string_view sv) {
        this->data = this->_parse_sv(std::string(sv));
        return *this;
    }
    SocketAddr& operator = (const char *addr) {
        this->data = this->_parse_sv(std::string(addr));
        return *this;
    }

    SocketAddr& operator = (struct sockaddr_in *addr) {
        this->data = *addr;
        return *this;
    }

    std::string ip() const;
    int port() const;

    struct sockaddr_in *as_sockaddr_in_ptr();
    struct sockaddr *as_sockaddr_ptr();
    socklen_t as_socklen_t() const;

private:
    struct sockaddr_in _parse_sv(std::string_view _sv) const;
    struct sockaddr_in _parse_ints(int a, int b, int c, int d, int port) const;

    struct sockaddr_in data;
};

}