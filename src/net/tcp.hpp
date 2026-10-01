#pragma once
#include <net/addrs.hpp>

namespace net {

class TcpStream {
public:
    TcpStream(const TcpStream&) = delete;
    TcpStream(TcpStream&& other) noexcept;

    ~TcpStream();

    TcpStream& operator = (const TcpStream&) = delete;
    TcpStream& operator = (TcpStream&&) noexcept;

    static TcpStream connect(net::SocketAddr conn);

    [[nodiscard]] SocketAddr peer_addr() const noexcept;

    ssize_t read(std::span<char> buffer) noexcept;
    ssize_t write(const std::span<char> buffer) noexcept;

    bool set_nonblocking(bool flag);
    [[nodiscard]] int descriptor() const noexcept;

    void close() noexcept;

    friend class TcpListener;
private:
    TcpStream(int _fd, SocketAddr _addr) : fd(_fd), peer(_addr) {};
    int fd;
    SocketAddr peer;
};


class TcpListener {
public:
    TcpListener(const TcpListener&) = delete;
    TcpListener(TcpListener&& other) noexcept;

    ~TcpListener();

    TcpListener& operator = (const TcpListener&) = delete;
    TcpListener& operator = (TcpListener&&) noexcept;

    [[nodiscard]] TcpStream accept();
    [[nodiscard]] static TcpListener bind(const SocketAddr& addr);

    // NO RECOMENDED!
    bool set_nonblocking(bool flag);
    [[nodiscard]] int descriptor() const noexcept;
    [[nodiscard]] SocketAddr address() const noexcept;
    void close() noexcept;

private:
    TcpListener(SocketAddr _addr, int _fd) : fd(_fd), addr(_addr) {};
    int fd;
    SocketAddr addr;
};

}