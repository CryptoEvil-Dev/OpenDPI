#pragma once

#include <utility>
#include <stdint.h>
#include <sock.hpp>

#include <vector>
#include <tuple>

namespace net {

enum TcpErrorKind : uint16_t {
    AddressAlreadyInUsed,
    PortAlreadyInUsed,
    BindError,
    ShutdownError,
};

class TcpStream {
public:
    TcpStream(int fd) : fd(fd) {};
    TcpStream(const TcpStream&) = delete;
    TcpStream(TcpStream &&other) {
        this->fd = std::move(other.fd);
    };

    size_t read_buf(std::string *buffer);
    size_t write_buf(const std::string_view *buffer) const;

    size_t read_bytes(std::vector<char> *buffer);
    size_t write_bytes(const std::vector<char> *buffer) const;

    TcpErrorKind shutdown();

private:
    int fd;
};
    
class TcpListener {
public:
    
    TcpListener(const TcpListener&) = delete;
    TcpListener(TcpListener&& other) = default;

    // Перед закрытием слушателя, нужно закрывать сокет
    ~TcpListener();

    static TcpErrorKind bind(SocketAddr sa);
    std::tuple<TcpStream, SocketAddr> accept();

private:
    TcpListener(SocketAddr sa);

    SocketAddr addr;
};

}
