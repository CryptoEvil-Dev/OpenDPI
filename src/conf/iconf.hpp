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


#define INSIDER_MODE
#ifdef INSIDER_MODE

/*
 * Универсальный валидатор.
 * Идея разделить извлечение данных (свой слой для каждого формата) и проверку типов (общий слой)
 * IConfigReader - базовый класс, который релизуют парсеры (YAML, JSON, TOML, XML)
 * SchemaValidator - Универсальный валидатор, принимающий парсер и валидирующий данные.
 * 
 * При таком подходе, от IConf можно будет отказаться.
 * 
 * Появилось предложение добавить Constraint'ы для первичного анализа данных, так будет ещё проще валидировать данные
 * 
*/

#include <optional>

template<typename T>
struct NumericConstraint {
    std::optional<T> min;
    std::optional<T> max;
    std::optional<T> equal;
};

struct StringConstraint {
    std::optional<size_t> max_length;
    std::optional<size_t> min_length;
    std::optional<std::string> is_contained;
};




class IConfigReader {
public:
    virtual ~IConfigReader() = default;
    virtual std::optional<std::string_view> get_value(std::string_view path) const = 0;
};


class SchemaValidator {
public:
    SchemaValidator() = default;
    SchemaValidator(const SchemaValidator&) = default;
    SchemaValidator(SchemaValidator&&) noexcept;

    SchemaValidator& operator = (const SchemaValidator&) = default;
    SchemaValidator& operator = (SchemaValidator&&) noexcept;

    template<typename T>
    SchemaValidator& check_anytype(const IConfigReader& reader, const char* path);

    // Without constraints
    template<typename T> requires std::integral<T>
    SchemaValidator& check_numeric(const IConfigReader& reader, const char* path);

    template<typename T> requires std::convertible_to<T, std::string_view>
    SchemaValidator& check_string(const IConfigReader& reader, const char* path);

    // With constraints
    template<typename T> requires std::integral<T>
    SchemaValidator& check_numeric(const IConfigReader& reader, const char* path, NumericConstraint<T> constraint);

    template<typename T> requires std::convertible_to<T, std::string_view>
    SchemaValidator& check_string(const IConfigReader& reader, const char* path, StringConstraint constraint);

    ValidationResult&& release();

private:
    ValidationResult _res;
};



template<typename T>
SchemaValidator& SchemaValidator::check_anytype(const IConfigReader& reader, const char* path) {
    auto val = reader.get_value(path);
    if(!val.has_value()) {
        this->_res.append(path, "is missing, empty or invalid");
        return *this;
    }
    try {
        T{*val};
    } catch(...) {
        this->_res.append(path, "invalid value");
    }
    return *this;
}

template<typename T> requires std::integral<T>
SchemaValidator& SchemaValidator::check_numeric(const IConfigReader& reader, const char* path) {
    auto val = reader.get_value(path);
    if(!val.has_value()) {
        this->_res.append(path, "is missing, empty or invalid");
        return *this;
    }
    T value = 0;
    auto [ptr, ec] = std::from_chars(val->data(), val->data() + val->size(), value);
    if(ec != std::errc{}) {
        this->_res.append(path, "invalid numeric value");
    }
    return *this;
}

template<typename T> requires std::convertible_to<T, std::string_view>
SchemaValidator& SchemaValidator::check_string(const IConfigReader& reader, const char* path) {
    auto val = reader.get_value(path);
    if(!val.has_value()) {
        this->_res.append(path, "is missing, empty or invalid");
    }
    return *this;
}

template<typename T> requires std::integral<T>
SchemaValidator& SchemaValidator::check_numeric(const IConfigReader& reader, const char* path, NumericConstraint<T> constraint) {
    auto val = reader.get_value(path);
    if(!val.has_value()) {
        this->_res.append(path, "is missing, empty or invalid");
        return *this;
    }
    T value = 0;
    auto [ptr, ec] = std::from_chars(val->data(), val->data() + val->size(), value);
    if(ec != std::errc{}) {
        this->_res.append(path, "invalid numeric value");
        return *this;
    }
    if(constraint.max && value > *constraint.max) {
        this->_res.append(path, "value is too big"); // Исправили "too bigger"
    }
    if(constraint.min && value < *constraint.min) {
        this->_res.append(path, "value is too small");
    }
    if(constraint.equal && value != *constraint.equal) {
        std::string err_msg;
        std::format_to(std::back_inserter(err_msg), "is not equal '{}'", *constraint.equal);
        this->_res.append(path, err_msg);
    }
    return *this;
}

template<typename T> requires std::convertible_to<T, std::string_view>
SchemaValidator& SchemaValidator::check_string(const IConfigReader& reader, const char* path, StringConstraint constraint) {
    auto val = reader.get_value(path);
    if(!val) {
        this->_res.append(path, "is missing, empty or invalid");
        return *this;
    }
    std::string_view sv = *val;
    if(constraint.min_length && sv.size() < *constraint.min_length) {
        this->_res.append(path, "string is too short");
    }
    if(constraint.max_length && sv.size() > *constraint.max_length) {
        this->_res.append(path, "string is too long");
    }
    if(constraint.is_contained && !sv.contains(*constraint.is_contained)) {
        std::string err_msg;
        std::format_to(std::back_inserter(err_msg), "must contain '{}'", *constraint.is_contained);
        this->_res.append(path, err_msg);
    }
    return *this;
}


#endif

}