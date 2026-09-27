#pragma once

#include <net/addrs.hpp>

#include <stdint.h>

#include <string>
#include <vector>

#include <chrono>

namespace conf {

// TODO!
/*
 * Спроектировать поля, которые будут передаваться в конфигурационном файле.
 * - Можно взять впринципе готовую конфигурацию из Rust репозитория, но изменить типы данных.
 * - Конфигурация не обязана шустро работать, но получать значения из неё нужно моментально.
 * - Реализовать сначала парсинг YAML конфигурации, потом можно будет реализовывать и иные форматы.
 *
 * 
*/
class IConf {
public:
    virtual ~IConf() = default;

    virtual net::SocketAddr dpi() const = 0;
    virtual net::SocketAddr backend() const = 0;

    // TLS
    virtual bool tls_enabled() const = 0; // Включен ли TLS.
    virtual std::string tls_pubkey_path() const = 0; // Путь к TLS-публичному ключу.
    virtual std::string tls_privkey_path() const = 0; // Путь к TLS-приватному ключу.
    virtual std::string tls_renewal_path() const = 0; // Путь к TLS-Renewal.

    // Anonymous TCP-Requests
    virtual std::chrono::milliseconds anonymous_ban_time() const = 0; // Время бана анонимных соединений.
    virtual std::chrono::milliseconds anonymous_ban_time_after_honeypot() const = 0; // Время бана анонимных соединений после перехода на honeypot.
    virtual uint64_t max_anonymous_packets_per_second() const = 0; // Максимальное количество пакетов от анонимного соединения в секунду.

    // Anonymous Web-Sockets
    virtual std::chrono::milliseconds anonymous_websocket_ban_time() const = 0; // Время бана анонимных Web-Sockets.


    // Other
    virtual std::vector<std::string> honeypots() const = 0; // Список honeypot маршрутов.
    virtual std::vector<net::IpAddr> angels() const = 0; // Список IP-адресов на которых не распространяются правила фильтрации.
    virtual std::vector<net::IpAddr> web_ui_for() const = 0; // Список IP-адресов, которым доступен веб-интерфейс.
};

}