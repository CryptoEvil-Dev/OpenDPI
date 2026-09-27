#include "tcp.hpp"
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <utility>

#include <system_error>
#include <vector>

namespace net {

TcpStream::~TcpStream() {
    if(this->fd != -1) ::close(this->fd);
}

TcpStream& TcpStream::operator = (TcpStream&& other) noexcept {
    if(this != &other) {
        if(this->fd != -1) ::close(this->fd);
        this->fd = std::exchange(other.fd, -1);
        this->peer = std::move(other.peer);
    }
    return *this;
}

TcpStream TcpStream::connect(net::SocketAddr conn) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(::connect(fd, &conn.as_sockaddr(), conn.as_socklen_t()) < 0) {
        int serr = errno;
        ::close(fd);
        switch(serr) {
            case ECONNREFUSED:
                throw std::errc::connection_refused;
            case ETIMEDOUT:
                throw std::errc::timed_out;
            case ENETUNREACH:
                throw std::errc::network_unreachable;
            case EHOSTUNREACH:
                throw std::errc::host_unreachable;
            case EADDRNOTAVAIL:
                throw std::errc::address_not_available;
            case EALREADY:
                throw std::errc::connection_already_in_progress;
            case EISCONN:
                throw std::errc::already_connected;
            case EACCES:
                throw std::errc::permission_denied;
            case EPERM:
                throw std::errc::permission_denied;
            case EAFNOSUPPORT:
                throw std::errc::address_family_not_supported;
            case EINTR:
                throw std::errc::interrupted;
            default:
                throw std::runtime_error("[UNDEF]: " + std::to_string(serr));
        }
    }

    return TcpStream(fd, conn);
}

SocketAddr TcpStream::peer_addr() const noexcept {
    return this->peer;
}


ssize_t TcpStream::read(std::span<char> buffer) noexcept {
    if(buffer.empty()) return 0;
    while(true) {
        ssize_t _rd = ::read(this->fd, buffer.data(), buffer.size());
        if(_rd < 0) [[unlikely]] {
            if(errno == EINTR) continue;
            return -1;
        }
        return _rd;
    }   
}

ssize_t TcpStream::write(const std::span<char> buffer) noexcept {
    if(buffer.empty()) return 0;

    while(true) {
        ssize_t _wd = ::write(this->fd, buffer.data(), buffer.size());

        if(_wd < 0) [[unlikely]] {
            if(errno == EINTR) continue;
            return -1;
        }

        return _wd;
    }
}


bool TcpStream::set_nonblocking(bool flag) {
    if(this->fd == -1) [[unlikely]] return false;

    int flags = ::fcntl(this->fd, F_GETFL, 0);
    if(flags == -1) [[unlikely]] return false;

    if(flag) flags |=  O_NONBLOCK;
    else     flags &= ~O_NONBLOCK;

    if(::fcntl(this->fd, F_SETFL, flags) == -1) [[unlikely]] return false;
    return true;
}

int TcpStream::descriptor() const noexcept {
    return this->fd;
}

void TcpStream::close() noexcept {
    if(this->fd != -1) {
        ::close(this->fd);
        this->fd = -1;
    }
}




TcpListener& TcpListener::operator = (TcpListener&& other) noexcept {
    if(this != &other) {
        if(this->fd != -1) ::close(this->fd);

        this->fd = std::exchange(other.fd, -1);
        this->addr = std::move(other.addr);
    }
    return *this;
}

TcpListener::~TcpListener() {
    if(this->fd != -1) ::close(this->fd);
}

TcpListener TcpListener::bind(const SocketAddr& addr) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0) throw std::errc::bad_file_descriptor;

    {
        int opt = 1;
        ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    if(::bind(fd, &addr.as_sockaddr(), addr.as_socklen_t()) < 0) {
        ::close(fd);
        throw std::errc::address_in_use;
    }

    if(::listen(fd, SOMAXCONN) < 0) {
        ::close(fd);
        throw std::errc::connection_refused;
    }
    
    return TcpListener(addr, fd);
}

TcpStream TcpListener::accept() {
    sockaddr_in _addr = {};
    socklen_t _addr_l = sizeof(_addr);

    int peer = ::accept(this->fd, reinterpret_cast<sockaddr*>(&_addr), &_addr_l);
    if(peer < 0) [[unlikely]] throw std::errc::connection_aborted;

    SocketAddr sa = _addr;

    return TcpStream(peer, sa);
}

// NO RECOMENDED!
bool TcpListener::set_nonblocking(bool flag) {
    if(this->fd == -1) [[unlikely]] return false;

    int flags = ::fcntl(this->fd, F_GETFL, 0);
    if(flags == -1) [[unlikely]] return false;

    if(flag) flags |=  O_NONBLOCK;
    else     flags &= ~O_NONBLOCK;

    if(::fcntl(this->fd, F_SETFL, flags) == -1) [[unlikely]] return false;
    return true;
}

int TcpListener::descriptor() const noexcept {
    return this->fd;
}

SocketAddr TcpListener::address() const noexcept {
    return this->addr;
}

void TcpListener::close() noexcept {
    if(this->fd != -1) {
        ::close(this->fd);
        this->fd = -1;
    }
}

}