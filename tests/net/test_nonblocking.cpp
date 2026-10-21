#include <gtest/gtest.h>
#include <net/tcp.hpp>
#include <sys/epoll.h>
#include <thread>
#include <vector>


TEST(TcpListener, MultiListeners) {
    constexpr int N = 100;

    // Первый listener — на порт 0, получаем реальный адрес
    auto first = net::TcpListener::bind(net::SocketAddr("127.0.0.1:0"));
    auto real_addr = first.address();
    ASSERT_NE(real_addr.port(), 0);
    first.set_nonblocking(true);

    // Остальные 99 — на тот же реальный порт
    std::vector<net::TcpListener> listeners;
    listeners.reserve(N - 1);
    listeners.push_back(std::move(first));
    for (int i = 1; i < N; ++i) {
        listeners.push_back(net::TcpListener::bind(real_addr));
        listeners.back().set_nonblocking(true);
        EXPECT_EQ(listeners.back().address().port(), real_addr.port());
    }

    // epoll с listener'ами
    int epfd = ::epoll_create1(EPOLL_CLOEXEC);
    ASSERT_NE(epfd, -1);

    for (auto& l : listeners) {
        epoll_event ev{};
        ev.events   = EPOLLIN;
        ev.data.ptr = &l;   // ← указатель на элемент ВЕКТОРА (живёт до конца теста)
        ASSERT_EQ(::epoll_ctl(epfd, EPOLL_CTL_ADD, l.descriptor(), &ev), 0);
    }

    // 100 клиентов параллельно
    std::atomic<int> accepted{0};
    std::vector<std::jthread> clients;
    clients.reserve(N);
    for (int i = 0; i < N; ++i) {
        clients.emplace_back([real_addr]() {
            auto conn = net::TcpStream::connect(real_addr);
            conn.close();
        });
    }

    // 5. Принимаем 100 соединений через epoll
    epoll_event events[100];
    int total = 0;
    while (total < N) {
        int nfds = ::epoll_wait(epfd, events, 100, 5000);   // 5 сек таймаут
        ASSERT_NE(nfds, -1);
        if (nfds == 0) break;   // таймаут — что-то не так

        for (int n = 0; n < nfds; ++n) {
            auto* l = static_cast<net::TcpListener*>(events[n].data.ptr);
            auto conn = l->accept();
            conn.close();
            total++;
        }
    }

    EXPECT_EQ(total, N);
    ::close(epfd);
}
