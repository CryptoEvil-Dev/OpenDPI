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

struct ValidationData {
    std::string path;
    std::string message;
};

class ValidationResult {
public:
    ValidationResult() = default;
    ValidationResult(const ValidationResult&) = default;
    ValidationResult(ValidationResult&&) noexcept;

    ValidationResult& operator = (const ValidationResult&) = default;
    ValidationResult& operator = (ValidationResult&&) noexcept;

    void append(std::string path, std::string message) noexcept;
    bool is_valid() const noexcept;

    std::string get_log() const noexcept;
    std::vector<ValidationData> get_raw_log() const noexcept;

private:
    std::vector<ValidationData> errors;
};

class IConf {
public:
    virtual ~IConf() = default;

    virtual void reload(const std::string& path) = 0;

    virtual net::SocketAddr dpi_addr() const noexcept = 0;
    virtual net::SocketAddr backend_addr() const noexcept = 0;

    // TLS
    virtual std::string pubkey() const noexcept = 0;
    virtual std::string privkey() const noexcept = 0;
    virtual std::string chainkey() const noexcept = 0;
    virtual bool renewal() const noexcept = 0;

    // Anonymous TCP
    virtual std::chrono::seconds ban_duration() const noexcept = 0;
    virtual uint64_t max_rps() const noexcept = 0;
    virtual std::chrono::seconds max_session_duration() const noexcept = 0;
    virtual uint64_t max_packet_size() const noexcept = 0;

private:
    virtual ValidationResult validate(const std::string& config) = 0;
};



#ifdef INSIDER_MODE

/*
 * Универсальный валидатор.
 * Идея разделить извлечение данных (свой слой для каждого формата) и проверку типов (общий слой)
 * IConfigReader - базовый класс, который релизуют парсеры (YAML, JSON, TOML, XML)
 * SchemaValidator - Универсальный валидатор, принимающий парсер и валидирующий данные
 * 
 * При таком подходе, от IConf можно будет отказаться
 * 
*/

#include <optional>

class IConfigReader {
public:
    virtual ~IConfigReader() = default;
    virtual std::optional<std::string_view> get_value(std::string_view path) const = 0;
};

#endif

}