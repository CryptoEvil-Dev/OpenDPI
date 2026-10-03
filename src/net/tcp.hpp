#pragma once
#include <net/addrs.hpp>

namespace net {


enum class IoStatus : uint8_t {
    Data,
    WouldBlock,
    Closed,
    Error,
};

struct IoResult {
    size_t   bytes  = 0;
    int      serr   = 0;
    IoStatus result = IoStatus::Data;

    constexpr bool ok() const noexcept { return result == IoStatus::Data; }
    constexpr bool would_block() const noexcept { return result == IoStatus::WouldBlock; }
    constexpr bool closed() const noexcept { return result == IoStatus::Closed; }
    constexpr bool error() const noexcept { return result == IoStatus::Error; }
};



class TcpStream {
public:
    TcpStream(const TcpStream&) = delete;
    TcpStream(TcpStream&& other) noexcept;

    ~TcpStream();

    TcpStream& operator = (const TcpStream&) = delete;
    TcpStream& operator = (TcpStream&&) noexcept;

    static TcpStream connect(net::SocketAddr conn);

    [[nodiscard]] SocketAddr peer_addr() const noexcept;

    IoResult read(std::span<char> buffer) noexcept;
    IoResult write(const std::span<char> buffer) noexcept;

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


static_assert(sizeof(IoStatus) <= 16, "IoStatus must fit in two register to avoid sret");


}