#include <gtest/gtest.h>
#include <net/tcp.hpp>
#include <thread>
#include <string>

// Универсальный тест: проверка отправки и чтения до закрытия сокета (вместо RouterConnect)
TEST(TcpStreamSend, LocalStreamConnect) {
    // Поднимаем временный сервер на случайном порту (например, 8086)
    net::TcpListener mock_server = net::TcpListener::bind(net::SocketAddr("127.0.0.1:8086"));

    // Имитируем поведение веб-сервера в фоновом потоке
    std::jthread server_thread([&mock_server]() {
        net::TcpStream client = mock_server.accept();
        std::string req;
        req.resize(1500);
        [[maybe_unused]] ssize_t rd = client.read(req); // вычитываем запрос
        
        // Отправляем длинный тестовый HTTP-ответ (имитация роутера)
        std::string mock_response = 
            "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n"
            + std::string(5000, 'A'); // Генерируем 5000 байт данных
        client.write(mock_response);
        client.close();
    });

    // Клиентская часть теста
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto conn = net::TcpStream::connect(net::SocketAddr("127.0.0.1:8086"));
    EXPECT_GE(conn.descriptor(), 0);

    std::string request = "GET / HTTP/1.1\r\nHost: 127.0.0.1:8086\r\nConnection: close\r\n\r\n";
    conn.write(request);

    std::string full_response;
    size_t total_bytes_received = 0;

    // Специфический момент: читаем чанками, пока сокет не закроется (read вернет <= 0)
    while (true) {
        std::string chunk;
        chunk.resize(1448); // Ограничение MTU / буфера
        ssize_t rd = conn.read(chunk);
        
        if (rd <= 0) break; // Сервер закрыл соединение — выходим штатно без зависаний!

        chunk.resize(static_cast<size_t>(rd)); // ИСПРАВЛЕНО: подрезаем под реальный размер!
        full_response.append(chunk);
        total_bytes_received += static_cast<size_t>(rd);
    }

    EXPECT_GT(total_bytes_received, 0);
    EXPECT_NE(full_response.find("200 OK"), std::string::npos); // Проверяем, что ответ валиден

    conn.close();
}

// Тест на чтение огромного буфера порциями (вместо RouterTest)
TEST(TcpStreamSend, StreamAccumulationTest) {
    net::TcpListener mock_server = net::TcpListener::bind(net::SocketAddr("127.0.0.1:8087"));

    std::jthread server_thread([&mock_server]() {
        net::TcpStream client = mock_server.accept();
        std::string req;
        req.resize(1500);
        [[maybe_unused]] ssize_t rd = client.read(req);
        
        // Отправляем гигантский массив данных порциями
        std::string data(20000, 'B'); 
        client.write(data);
        client.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto conn = net::TcpStream::connect(net::SocketAddr("127.0.0.1:8087"));
    EXPECT_GE(conn.descriptor(), 0);

    std::string request = "GET /login HTTP/1.1\r\nConnection: close\r\n\r\n";
    conn.write(request);

    std::string output; // ИСПРАВЛЕНО: убран некорректный resize(50000)
    std::string bf;
    
    while (true) {
        bf.clear();
        bf.resize(1500);
        ssize_t readed = conn.read(bf);
        if (readed <= 0) break;

        bf.resize(static_cast<size_t>(readed)); // ИСПРАВЛЕНО: убираем мусорные нули
        output.append(bf);
    }

    EXPECT_EQ(output.size(), 20000); // Гарантируем, что долетел каждый байтик
    EXPECT_EQ(output[0], 'B');

    conn.close();
}

// Ваш идеальный тест листенера, теперь полностью безопасный
TEST(TcpListenerTest, TestListener) {
    net::TcpListener listener = net::TcpListener::bind(net::SocketAddr("127.0.0.1:8085")); // Использован 127.0.0.1

    std::jthread connecter([](){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        net::TcpStream conn = net::TcpStream::connect("127.0.0.1:8085");
        std::string buffer = "Server Hello";
        conn.write(buffer);
        buffer.clear();
        buffer.resize(1500);
        ssize_t readed = conn.read(buffer);
        conn.close();
        buffer.resize(static_cast<size_t>(readed));
        EXPECT_EQ(buffer, "Client Hello");
    });

    net::TcpStream conn = listener.accept();
    std::string buffer;
    buffer.resize(1500);
    ssize_t readed = conn.read(buffer);
    buffer.resize(static_cast<size_t>(readed));
    EXPECT_EQ(buffer, "Server Hello");
    buffer = "Client Hello";
    conn.write(buffer);
    conn.close();
}