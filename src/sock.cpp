#include <sock.hpp>


struct sockaddr_in net::SocketAddr::_parse_sv(std::string_view sv) const {
    size_t delidx = sv.find(':');
    if(delidx == std::string_view::npos) {
        std::perror("Address is incomplete");
        return sockaddr_in{0};
    }
    std::string ip = std::string(sv.substr(0, delidx));


    struct sockaddr_in sa = {0};

    if(inet_pton(AF_INET, ip.c_str(), &sa.sin_addr) < 1) {
        std::perror("Invalid address");
        return sockaddr_in{0};
    }
    sa.sin_family = AF_INET;
    sa.sin_port = htons(std::stoi(std::string(sv.substr(delidx +1))));
    return sa;
}

struct sockaddr_in net::SocketAddr::_parse_ints(int a, int b, int c, int d, int port) const {
    sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    uint32_t addr = ((uint32_t)a & 0xFF) << 24 |
                    ((uint32_t)b & 0xFF) << 16 |
                    ((uint32_t)c & 0xFF) <<  8 |
                    ((uint32_t)d & 0xFF);
    sa.sin_addr.s_addr = htonl(addr);
    return sa;
}

std::string net::SocketAddr::ip() const {
    char ip[INET_ADDRSTRLEN];

    if(inet_ntop(AF_INET, &(this->data.sin_addr), ip, INET_ADDRSTRLEN) == nullptr) {
        std::perror("Failed to convert address");
        return std::string("");
    }
    return std::string(ip);
}

int net::SocketAddr::port() const {
    return ntohs(this->data.sin_port);
}

struct sockaddr_in *net::SocketAddr::as_sockaddr_in_ptr() {
    return &this->data;
}

struct sockaddr* net::SocketAddr::as_sockaddr_ptr() {
    return reinterpret_cast<sockaddr*>(&this->data);
}

socklen_t net::SocketAddr::as_socklen_t() const {
    return socklen_t(sizeof(this->data));
}