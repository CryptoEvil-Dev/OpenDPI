#include <gtest/gtest.h>
#include <net/tcp.hpp>
#include <event/events.hpp>

namespace {

// Локальный класс выносим наружу функции (иначе нельзя использовать
// в шаблонах gtest-макросов, и вообще читать проще).
class AcceptOnceHandler final : public event::IEventHandler {
public:
    explicit AcceptOnceHandler(net::TcpListener& l) : listener_(l) {}

    void on_read(event::EventCtl&) override {
        try {
            net::TcpStream stream = listener_.accept();
            stream.close();
            ++accepted_;
        } catch (const std::system_error& e) {
            if (e.code().value() != EAGAIN && e.code().value() != EWOULDBLOCK)
                throw;
        }
    }
    void on_write(event::EventCtl&) override {}
    void on_hup(event::EventCtl&)   override {}
    void on_err(event::EventCtl&)   override {}

    int fd() const override { return listener_.descriptor(); }

    int accepted() const { return accepted_; }

private:
    net::TcpListener& listener_;
    int accepted_ = 0;
};

} // namespace

TEST(TestEventCtl, AddDelModSetUnset) {
    event::EventCtl evctl;
    net::TcpListener listener = net::TcpListener::bind(net::SocketAddr("127.0.0.1:0"));
    ASSERT_TRUE(listener.set_nonblocking(true));

    AcceptOnceHandler handler(listener);   // живёт до конца теста — ок

    EXPECT_NO_THROW(evctl.add(listener.descriptor(), EPOLLIN, handler));

    // Проверяем mod/set/unset — они не бросают на валидном fd.
    EXPECT_NO_THROW(evctl.set  (listener.descriptor(), EPOLLOUT));
    EXPECT_NO_THROW(evctl.unset(listener.descriptor(), EPOLLOUT));
    EXPECT_NO_THROW(evctl.mod  (listener.descriptor(), EPOLLIN));

    // del до run() — безопасно.
    EXPECT_NO_THROW(evctl.del(listener.descriptor()));

    // Повторный del должен бросить (ENOENT -> runtime_error).
    EXPECT_THROW(evctl.del(listener.descriptor()), std::runtime_error);
}