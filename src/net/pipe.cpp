#include <net/pipe.hpp>

const int net::Pipe::client_fd() const noexcept { return this->_client.descriptor(); }
const int net::Pipe::backend_fd() const noexcept { return this->_backend.descriptor(); }
net::Pipe::State net::Pipe::state() const noexcept { return this->_state(); }
event::IEventHandler* net::Pipe::client_handler() noexcept { return &_client_handler; }
event::IEventHandler* net::Pipe::backend_handler() noexcept { return &_backend_handler; }

