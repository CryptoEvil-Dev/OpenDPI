#include <event/events.hpp>
#include <sys/eventfd.h>
#include <sys/resource.h>
#include <stdexcept>
#include <cstdint>
#include <cerrno>
#include <unistd.h>

namespace event {

EventCtl::EventCtl() {
    this->events.resize(64);
    {
        struct rlimit rl;
        if(getrlimit(RLIMIT_NOFILE, &rl) == 0) {
            this->local_ev.resize(static_cast<size_t>(rl.rlim_cur));
        } else {
            this->local_ev.resize(65535);
        }
    }

    // this->local_ev.resize(max_events);
    this->epfd = epoll_create1(EPOLL_CLOEXEC);
    if(this->epfd == -1) {
        throw std::runtime_error("epoll_create1 failed");
    }

    this->wakeupfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    struct ::epoll_event ev {};
    ev.events = EPOLLIN;
    ev.data.ptr = nullptr;
    if(epoll_ctl(this->epfd, EPOLL_CTL_ADD, this->wakeupfd, &ev) == -1) {
        throw std::runtime_error("epoll_ctl: EPOLL_CTL_ADD wakeup fd error");
    }
}

EventCtl::~EventCtl() {
    if(this->wakeupfd != -1) ::close(this->wakeupfd);
    if(this->epfd != -1) ::close(this->epfd);
}

void EventCtl::add(int fd, uint32_t flags, IEventHandler& callback) {
    if(fd < 0 || static_cast<size_t>(fd) >= this->local_ev.size()) {
        throw std::out_of_range("fd out of range");
    }
    struct ::epoll_event ev{};
    ev.data.ptr = &callback;
    ev.events = flags;
    if(epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        throw std::runtime_error("epoll_ctl: EPOLL_CTL_ADD error");
    }
    this->local_ev[static_cast<size_t>(fd)] = ev;
}

void EventCtl::del(int fd) {
    if(fd < 0 || static_cast<size_t>(fd) >= this->local_ev.size()) {
        throw std::out_of_range("fd out of range");
    }
    if(epoll_ctl(this->epfd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
        throw std::runtime_error("epoll_ctl: EPOLL_CTL_DEL error");
    }
    this->local_ev[static_cast<size_t>(fd)] = {};
}

void EventCtl::mod(int fd, uint32_t flag) {
    if(fd < 0 || static_cast<size_t>(fd) >= this->local_ev.size()) {
        throw std::out_of_range("fd out of range");
    }
    struct ::epoll_event ev = this->local_ev[static_cast<size_t>(fd)];
    ev.events = flag;
    if(epoll_ctl(this->epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
        throw std::runtime_error("epoll_ctl: EPOLL_CTL_MOD error");
    }
    this->local_ev[static_cast<size_t>(fd)] = ev;
}

void EventCtl::set(int fd, uint32_t flag) {
    if(fd < 0 || static_cast<size_t>(fd) >= this->local_ev.size()) {
        throw std::out_of_range("fd out of range");
    }
    struct ::epoll_event ev = this->local_ev[static_cast<size_t>(fd)];
    ev.events |= flag;
    if(epoll_ctl(this->epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
        throw std::runtime_error("epoll_ctl: EPOLL_CTL_MOD error");
    }
    this->local_ev[static_cast<size_t>(fd)] = ev;
}

void EventCtl::unset(int fd, uint32_t flag) {
    if(fd < 0 || static_cast<size_t>(fd) >= this->local_ev.size()) {
        throw std::out_of_range("fd out of range");
    }
    struct ::epoll_event ev = this->local_ev[static_cast<size_t>(fd)];
    ev.events &= ~flag;
    if(epoll_ctl(this->epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
        throw std::runtime_error("epoll_ctl: EPOLL_CTL_MOD error");
    }
    this->local_ev[static_cast<size_t>(fd)] = ev;
}

void EventCtl::run() {
    this->running = true;
    while(this->running) {
        int nfds = epoll_wait(epfd, this->events.data(), static_cast<int>(this->events.size()), -1);
        if(nfds == -1) {
            if(errno == EINTR) continue;
            throw std::runtime_error("epoll_wait error");
        }
            for(size_t n = 0; n < static_cast<size_t>(nfds); ++n) {
                // Сначала проверяем wakeup fd, через него мы будем управлять EventCtl
                if(this->events[n].data.ptr == nullptr) {
                    uint64_t val;
                    ::read(this->wakeupfd, &val, sizeof(val));
                    this->running = false;
                    continue;
                }

                auto *h = static_cast<IEventHandler*>(this->events[n].data.ptr);
                uint32_t ev = this->events[n].events;
                if(ev & EPOLLIN) h->on_read(*this);
                if(ev & EPOLLOUT) h->on_write(*this);
                if(ev & EPOLLHUP) h->on_hup(*this);
                if(ev & EPOLLERR) h->on_err(*this);
            }
    }
}

void EventCtl::stop() {
    if(this->wakeupfd == -1) return;
    uint64_t u = 1;
    (void)::write(this->wakeupfd, &u, sizeof(uint64_t));
}

}
