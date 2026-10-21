#pragma once


#include <sys/epoll.h>
#include <memory>
#include <vector>

namespace event {

class IEventHandler;

class EventCtl {
public:
    EventCtl();
    EventCtl(const EventCtl&) = delete;
    EventCtl(EventCtl&&) = delete;
    ~EventCtl();

    EventCtl& operator = (const EventCtl&) = delete;
    EventCtl& operator = (EventCtl&&) = delete;

    void add(int fd, uint32_t flags, IEventHandler& callback);
    void del(int fd);
    void mod(int fd, uint32_t flags);

    void set(int fd, uint32_t flag);
    void unset(int fd, uint32_t flag);

    void run();
    void stop();

private:
    int epfd;
    int wakeupfd;
    bool running = false;
    std::vector<struct ::epoll_event> events;
    std::vector<struct ::epoll_event> local_ev;
};

class IEventHandler {
public:
    virtual void on_read(EventCtl&  ctl) = 0;
    virtual void on_write(EventCtl& ctl) = 0;
    virtual void on_hup(EventCtl&   ctl) = 0;
    virtual void on_err(EventCtl&   ctl) = 0;

    virtual int fd() const = 0;

    virtual ~IEventHandler() = default;
};

}