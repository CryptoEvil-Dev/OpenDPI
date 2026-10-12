#pragma once

#include <net/tcp.hpp>

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <memory>

namespace net {


struct Pipe {
    TcpStream client;
    TcpStream backend;

    // RX: client -> backend
    std::vector<char> rx_buffer;
    size_t rx_pos = 0;

    // TX: backend -> client
    std::vector<char> tx_buffer;
    size_t tx_pos = 0;

    enum class State { Analyzing, Proxying, Blocked, Closed, Connecting };
    State state = State::Analyzing;
};



class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop(EventLoop&&) = delete;
    EventLoop& operator = (const EventLoop&) = delete;
    EventLoop& operator = (EventLoop&&) = delete;

    void add_listener(TcpListener& listener);
    void run();
    void stop();
    
private:
    enum class FdType : uint8_t { Free, Listener, Client, Backend };

    void on_accept();
    void on_client_readable(Pipe& pipe);
    void on_backend_readable(Pipe& pipe);
    void close_pipe(Pipe& pipe);

    void register_fd(int fd, FdType type);
    void unregister_fd(int fd);

    int _epoll_fd = -1;
    int _wakeup_fd = -1; // eventfd для stop()

    TcpListener* _listener = nullptr;
    int _listener_fd = -1;

    std::vector<std::unique_ptr<Pipe>> _pipes; // индекс = fd
    std::vector<FdType> _fd_types;

    bool _running = false;

    net::SocketAddr _backend_addr;
};

}