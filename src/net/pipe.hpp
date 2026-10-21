#pragma once

#include <memory>
#include <net/tcp.hpp>
#include <event/events.hpp>

namespace net {

class Pipe {
public:
    Pipe(TcpStream client, net::TcpStream backend) : _client(std::move(client)), _backend(std::move(backend)) {};
    Pipe(const Pipe&) = delete;
    Pipe(Pipe&&) = delete;

    Pipe& operator = (const Pipe&) = delete;
    Pipe& operator = (Pipe&&) = delete;

    void on_client_read();
    void on_client_write();
    void on_backend_read();
    void on_backend_write();
    void on_error();
    void on_hup();

    const int client_fd() const noexcept;
    const int backend_fd() const noexcept { return this->backend.descriptor(); }

    enum class State : uint8_t { Connecting, Proxying, Blocked, Closed };
    State state() const noexcept { return this->_state; }

    class SideHandler : public event::IEventHandler {
    public:
        enum class Side : uint8_t { Client, Backend };
        SideHandler(Pipe* pipe, Side side) : _pipe(pipe), _side(side) {}
        void on_read() override;
        void on_write() override;
        void on_err() override;
        void on_hup() override;
        const int fd() const override;
    private:
        Pipe* _pipe;
        Side _side;
    };

    event::IEventHandler* client_handler() noexcept { return &_client_handler; }
    event::IEventHandler* backend_handler() noexcept { return &_backend_handler; }

    // При закрытии Pipe, должны закрываться соединения внтури Pipe
    ~Pipe();

private:
    net::TcpStream _client;
    net::TcpStream _backend;
    std::array<std::byte, 1500> _rx {}; // from client
    size_t _rx_filled = 0;
    std::array<std::byte, 1500> _tx {}; // from backend
    size_t _tx_filled = 0;

    State _state = State::Connecting;

    SideHandler _client_handler{this, SideHandler::Side::Client};
    SideHandler _backend_handler{this, SideHandler::Side::Backend};
};

}