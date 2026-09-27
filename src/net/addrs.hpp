#pragma once

#include <string>
#include <stdint.h>
#include <arpa/inet.h>
#include <compare>
#include <ostream>

namespace net {
class IpAddr;

class SocketAddr {
public:
    SocketAddr();

    SocketAddr(const uint32_t raw, const uint16_t port);
    SocketAddr(const sockaddr_in& addr);
    SocketAddr(const sockaddr& addr);
    SocketAddr(const std::string_view sva);
    SocketAddr(const char* cstr_addr) : SocketAddr(std::string_view { cstr_addr }) {};
    SocketAddr(const IpAddr& addr, int port);

    SocketAddr(const SocketAddr&) = default;
    SocketAddr(SocketAddr&&) noexcept = default;

    SocketAddr& operator = (const SocketAddr&) = default;
    SocketAddr& operator = (SocketAddr&&) noexcept = default;


    SocketAddr& operator = (const std::string_view sva);
    SocketAddr& operator = (const char* cstr_addr);

    bool operator == (const SocketAddr& other) const;
    std::strong_ordering operator <=> (const SocketAddr&) const;

    [[nodiscard]] std::string ip() const;
    [[nodiscard]] int port() const;

    [[nodiscard]] sockaddr_in* as_sockaddr_in();
    [[nodiscard]] const sockaddr& as_sockaddr() const;
    [[nodiscard]] socklen_t as_socklen_t() const;

    friend std::ostream& operator << (std::ostream& os, const SocketAddr& saddr) {
        os << saddr.ip() << ":" << saddr.port();
        return os;
    }

private:
    sockaddr_in __addr;
};


class IpAddr {
public:
    IpAddr() : __addr(0) {};
    IpAddr(const IpAddr& other) : __addr(other.__addr) {};
    IpAddr(IpAddr&& other) noexcept : __addr(std::move(other.__addr)) {};

    IpAddr(const SocketAddr& sa);
    IpAddr(const std::string_view sva);
    IpAddr(const char* cstr_addr) : IpAddr(std::string_view(cstr_addr)) {};

    IpAddr& operator = (const IpAddr&);
    IpAddr& operator = (IpAddr&&) noexcept;

    auto operator <=> (const IpAddr&) const = default;

    [[nodiscard]] std::string ip() const;

    friend std::ostream& operator << (std::ostream& os, const IpAddr& ipa) {
        os << ipa.ip();
        return os;
    }
private:
    uint32_t __addr;
};

}