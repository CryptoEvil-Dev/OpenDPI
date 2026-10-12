#include <net/events.hpp>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <system_error>

namespace net {

// ============================================================================
// Конструктор / деструктор
// ============================================================================

EventLoop::EventLoop() {
    _epoll_fd = ::epoll_create1(EPOLL_CLOEXEC);
    if (_epoll_fd < 0) {
        throw std::system_error(errno, std::generic_category(), "epoll_create1");
    }

    _wakeup_fd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (_wakeup_fd < 0) {
        int err = errno;
        ::close(_epoll_fd);
        _epoll_fd = -1;
        throw std::system_error(err, std::generic_category(), "eventfd");
    }

    // Регистрируем wakeup_fd в epoll
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = _wakeup_fd;
    if (::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _wakeup_fd, &ev) < 0) {
        int err = errno;
        ::close(_wakeup_fd);
        ::close(_epoll_fd);
        _wakeup_fd = -1;
        _epoll_fd = -1;
        throw std::system_error(err, std::generic_category(), "epoll_ctl(wakeup_fd)");
    }
}

EventLoop::~EventLoop() {
    // Закрываем все Pipe
    for (auto& pipe : _pipes) {
        pipe.reset();   // деструктор Pipe закроет fd клиента и backend
    }
    _pipes.clear();
    _fd_types.clear();

    if (_wakeup_fd != -1) ::close(_wakeup_fd);
    if (_epoll_fd != -1)  ::close(_epoll_fd);
}

// ============================================================================
// Регистрация слушателя
// ============================================================================

void EventLoop::add_listener(TcpListener& listener) {
    if (_listener != nullptr) {
        throw std::runtime_error("EventLoop already has a listener");
    }
    _listener = &listener;
    _listener_fd = listener.descriptor();

    // Регистрируем listener в epoll
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = _listener_fd;
    if (::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _listener_fd, &ev) < 0) {
        _listener = nullptr;
        _listener_fd = -1;
        throw std::system_error(errno, std::generic_category(), "epoll_ctl(listener)");
    }

    // Расширяем _fd_types, чтобы индекс listener_fd был валидным
    register_fd(_listener_fd, FdType::Listener);
}

// ============================================================================
// Вспомогательные методы
// ============================================================================

void EventLoop::register_fd(int fd, FdType type) {
    // Расширяем векторы, если нужно
    if (static_cast<size_t>(fd) >= _fd_types.size()) {
        _fd_types.resize(fd + 1, FdType::Free);
        _pipes.resize(fd + 1);
    }
    _fd_types[fd] = type;
}

void EventLoop::unregister_fd(int fd) {
    if (fd < 0 || static_cast<size_t>(fd) >= _fd_types.size()) return;
    _fd_types[fd] = FdType::Free;
    _pipes[fd].reset();
}

// ============================================================================
// Основной цикл
// ============================================================================

void EventLoop::run() {
    _running = true;
    std::vector<epoll_event> events(64);

    while (_running) {
        int n = ::epoll_wait(_epoll_fd, events.data(), events.size(), -1);
        if (n < 0) {
            if (errno == EINTR) continue;
            throw std::system_error(errno, std::generic_category(), "epoll_wait");
        }

        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;

            // Обработка wakeup_fd (stop)
            if (fd == _wakeup_fd) {
                uint64_t val;
                ::read(_wakeup_fd, &val, sizeof(val));   // сбрасываем eventfd
                continue;
            }

            // Защита от выхода за границы
            if (static_cast<size_t>(fd) >= _fd_types.size()) continue;

            FdType type = _fd_types[fd];

            switch (type) {
                case FdType::Listener:
                    on_accept();
                    break;

                case FdType::Client: {
                    Pipe* pipe = _pipes[fd].get();
                    if (pipe) on_client_readable(*pipe);
                    break;
                }

                case FdType::Backend: {
                    Pipe* pipe = _pipes[fd].get();
                    if (pipe) on_backend_readable(*pipe);
                    break;
                }

                case FdType::Free:
                    // stale event — уже закрыт, игнорируем
                    break;
            }
        }
    }
}

// ============================================================================
// Остановка
// ============================================================================

void EventLoop::stop() {
    _running = false;
    if (_wakeup_fd != -1) {
        uint64_t val = 1;
        ::write(_wakeup_fd, &val, sizeof(val));
    }
}

// ============================================================================
// Обработчики (пока заглушки — заполним на следующем шаге)
// ============================================================================

void EventLoop::on_accept() {
    // TODO: реализуем на следующем шаге
    TcpStream client = _listener->accept();

    auto pipe = std::make_unique<Pipe>();
    pipe->client = std::move(client);
    pipe->state = Pipe::State::Connecting;

    try {
        pipe->backend = TcpStream::connect_nonblocking(_backend_addr);
    } catch(const std::system_error& ex) {
        close_pipe(*pipe);
        return;
    }

    const int cfd = pipe->client.descriptor();
    const int bfd = pipe->backend.descriptor();

    register_fd(cfd, FdType::Client);
    register_fd(bfd, FdType::Backend);
    _pipes[cfd] = std::move(pipe);

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = cfd;
    ::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, cfd, &ev);

    ev.events = EPOLLOUT;
    ev.data.fd = bfd;
    epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, bfd, &ev);

}

void EventLoop::on_client_readable(Pipe& pipe) {
    // TODO: реализуем на следующем шаге
}

void EventLoop::on_backend_readable(Pipe& pipe) {
    // TODO: реализуем на следующем шаге
}

void EventLoop::close_pipe(Pipe& pipe) {
    // TODO: реализуем на следующем шаге
}

} // namespace net