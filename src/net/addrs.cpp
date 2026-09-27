#include "addrs.hpp"
#include <algorithm>
#include <netdb.h>
#include <charconv>
#include <cstring>

namespace net {
    namespace {
        sockaddr_in parse_addr_impl(const std::string_view sva) {
            sockaddr_in saddr {};
            size_t delim = sva.find(':');

            std::string addr_part { sva.substr(0, delim) };
            std::string_view port_view = sva.substr(delim +1);

            saddr.sin_family = AF_INET;
            {
                int pton_stat = inet_pton(AF_INET, addr_part.c_str(), &saddr.sin_addr);
                if(pton_stat == 0) {
                    if(!addr_part.empty() && addr_part.back() == '.')
                        throw std::invalid_argument("IP address cannot end with a dot");
                    
                    std::ptrdiff_t dots = std::count(addr_part.begin(), addr_part.end(), '.');
                    if(dots != 3)
                        throw std::invalid_argument("Invalid IP address format: expected 4 octets");
                    
                    throw std::out_of_range("IP octet is out of range (0 - 255)");
                }
                else if(pton_stat < 0) throw std::invalid_argument("Unsupported address family");
            }
            
            {
                int port = 0;
                auto [ptr, ec] = std::from_chars(port_view.data(), port_view.data() + port_view.size(), port);
                if(ec != std::errc{} || ptr != port_view.data() + port_view.size() || port < 0 || port > 65535)
                    throw std::out_of_range("port is out of range");
                saddr.sin_port = htons(static_cast<uint16_t>(port));
            }

            return saddr;
        }

        sockaddr_in parse_dns_impl(const std::string_view sva) {
            sockaddr_in saddr {};
            size_t delim = sva.find(':');

            std::string host_part { sva.substr(0, delim) };
            std::string_view port_view { sva.substr(delim +1) };

            int port = 0;
            auto [ptr, ec] = std::from_chars(port_view.data(), port_view.data() + port_view.size(), port);
            if(ec != std::errc{} || ptr != port_view.data() + port_view.size() || port < 0 || port > 65535)
                throw std::out_of_range("port is out of range");
            
            
            addrinfo hints {};
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;

            addrinfo* result = nullptr;

            int dns_ec = getaddrinfo(host_part.c_str(), nullptr, &hints, &result);
            if(dns_ec != 0) {
                throw std::invalid_argument("failed to resolve domain name");
            }

            if(result && result->ai_addr && result->ai_addrlen >= sizeof(sockaddr_in)){
                std::memcpy(&saddr, result->ai_addr, sizeof(sockaddr_in));
            } else {
                if(result) freeaddrinfo(result);
                    throw std::invalid_argument("resolved address is invalid");
            }

            freeaddrinfo(result);
            saddr.sin_port = htons(static_cast<uint16_t>(port));
            return saddr;
        }
    }

    SocketAddr::SocketAddr(const uint32_t raw, const uint16_t port) {
        this->__addr = {};
        this->__addr.sin_family = AF_INET;
        this->__addr.sin_addr.s_addr = raw;
        this->__addr.sin_port = htons(port);
    }

    SocketAddr::SocketAddr() {
        this->__addr = {};
        this->__addr.sin_family = AF_INET;
        this->__addr.sin_addr.s_addr = INADDR_ANY;
        this->__addr.sin_port = htons(0);
    }

    SocketAddr::SocketAddr(const sockaddr_in& addr) {
        if(addr.sin_family == AF_INET)
            this->__addr = addr;
        else throw std::invalid_argument("AF_INET required!");
    }

    SocketAddr::SocketAddr(const sockaddr& addr) {
        if(addr.sa_family == AF_INET)
            this->__addr = *reinterpret_cast<const sockaddr_in*>(&addr);
        else throw std::invalid_argument("AF_INET required!");
    }

    SocketAddr::SocketAddr(const std::string_view sva) {
        size_t delim = sva.find(':');
        if(delim == std::string_view::npos || delim == 0 || delim == sva.size() - 1)
            throw std::invalid_argument("port number required!");
        
        std::string_view addr_part = sva.substr(0, delim);

        bool is_pure = std::all_of(addr_part.begin(), addr_part.end(), [](unsigned char c) {
            return std::isdigit(c) || c == '.';
        });

        if(!is_pure) this->__addr = parse_dns_impl(sva);
        else this->__addr = parse_addr_impl(sva);
    }

    
    SocketAddr::SocketAddr(const IpAddr& addr, int port) {
        std::string saddr = addr.ip();
        saddr.append(":" + std::to_string(port));
        *this = SocketAddr(saddr);
    }

    SocketAddr& SocketAddr::operator = (const std::string_view sva) {
        *this = SocketAddr(sva);
        return *this;
    }

    SocketAddr& SocketAddr::operator = (const char* cstr_addr) {
        *this = SocketAddr(std::string_view { cstr_addr });
        return *this;
    }

    bool SocketAddr::operator == (const SocketAddr& other) const {
        return this->__addr.sin_addr.s_addr == other.__addr.sin_addr.s_addr &&
                this->__addr.sin_port == other.__addr.sin_port;
    }

    std::strong_ordering SocketAddr::operator <=> (const SocketAddr& other) const {
        uint32_t lhs_ip = ntohl(this->__addr.sin_addr.s_addr);
        uint32_t rhs_ip = ntohl(other.__addr.sin_addr.s_addr);
        if(auto cmp = lhs_ip <=> rhs_ip; cmp != 0) return cmp;

        uint16_t lhs_port = ntohs(this->__addr.sin_port);
        uint16_t rhs_port = ntohs(other.__addr.sin_port);
        return lhs_port <=> rhs_port;
    }

    std::string SocketAddr::ip() const {
        char buf[INET_ADDRSTRLEN];
        if(inet_ntop(AF_INET, &this->__addr.sin_addr, buf, sizeof(buf)))
            return std::string(buf);
        return "0.0.0.0";
    }

    int SocketAddr::port() const {
        return ntohs(this->__addr.sin_port);
    }

    sockaddr_in* SocketAddr::as_sockaddr_in() {
        return &this->__addr;
    }

    const sockaddr& SocketAddr::as_sockaddr() const {
        return *reinterpret_cast<const sockaddr*>(&this->__addr);
    }

    socklen_t SocketAddr::as_socklen_t() const {
        return sizeof(this->__addr);
    }


// IpAddr

    IpAddr::IpAddr(const SocketAddr& sa) {
        const sockaddr_in *raw = const_cast<SocketAddr&>(sa).as_sockaddr_in();
        this->__addr = raw->sin_addr.s_addr;
    }

    IpAddr::IpAddr(const std::string_view sva) {
        sockaddr_in saddr {};
        size_t delim = sva.find(':');

        std::string_view clean_part = (delim == std::string_view::npos) ? sva : sva.substr(0, delim);
        bool is_pure = std::all_of(clean_part.begin(), clean_part.end(), [](unsigned char c) {
            return std::isdigit(c) || c == '.';
        });

        if(delim == std::string_view::npos) {
            std::string addr {sva};
            addr.append(":0");
            if(is_pure) saddr = parse_addr_impl(addr);
            else saddr = parse_dns_impl(addr);
        } else {
            if(is_pure) saddr = parse_addr_impl(sva);
            else saddr = parse_dns_impl(sva);
        }

        this->__addr = saddr.sin_addr.s_addr;
    }

    IpAddr& IpAddr::operator = (const IpAddr& other) {
        if(this != &other) {
            this->__addr = other.__addr;
        }
        return *this;
    }

    IpAddr& IpAddr::operator = (IpAddr&& other) noexcept {
        if(this != &other) {
            this->__addr = std::move(other.__addr);
        }
        return *this;
    }

    std::string IpAddr::ip() const {
        char buf[INET_ADDRSTRLEN];
        if(inet_ntop(AF_INET, &this->__addr, buf, sizeof(buf)))
            return std::string(buf);
        return "0.0.0.0";
    }

}
