#pragma once
#include <net/addrs.hpp>
#include <stdint.h>
#include <string>
#include <vector>
#include <optional>

namespace conf {

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
    virtual std::optional<std::vector<std::string_view>> get_sequence(std::string_view path) const = 0;
};


class SchemaValidator {
public:
    SchemaValidator() = default;
    // SchemaValidator(const SchemaValidator&) = default;
    SchemaValidator(SchemaValidator&&) noexcept;

    SchemaValidator& operator = (const SchemaValidator&) = default;
    SchemaValidator& operator = (SchemaValidator&&) noexcept;

    template<typename T>
    SchemaValidator& check_anytype(const IConfigReader& reader, const char* path);

    // Without constraints
    template<typename T> requires std::integral<T>
    SchemaValidator& check_numeric(const IConfigReader& reader, const char* path);

    // template<typename T> requires std::convertible_to<T, std::string_view>
    SchemaValidator& check_string(const IConfigReader& reader, const char* path);

    // With constraints
    template<typename T> requires std::integral<T>
    SchemaValidator& check_numeric(const IConfigReader& reader, const char* path, NumericConstraint<T> constraint);

    // template<typename T> requires std::convertible_to<T, std::string_view>
    SchemaValidator& check_string(const IConfigReader& reader, const char* path, StringConstraint constraint);

    ValidationResult release();

private:
    ValidationResult _res;
};



template<typename T>
inline SchemaValidator& SchemaValidator::check_anytype(const IConfigReader& reader, const char* path) {
    auto val = reader.get_value(path);
    if(!val.has_value()) {
        this->_res.append(path, "is missing, empty or invalid");
        return *this;
    }
    try {
        T{std::string(*val)};
    } catch(...) {
        this->_res.append(path, "invalid value");
    }
    return *this;
}

template<typename T> requires std::integral<T>
inline SchemaValidator& SchemaValidator::check_numeric(const IConfigReader& reader, const char* path) {
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

// template<typename T> requires std::convertible_to<T, std::string_view>
inline SchemaValidator& SchemaValidator::check_string(const IConfigReader& reader, const char* path) {
    auto val = reader.get_value(path);
    if(!val.has_value()) {
        this->_res.append(path, "is missing, empty or invalid");
    }
    return *this;
}

template<typename T> requires std::integral<T>
inline SchemaValidator& SchemaValidator::check_numeric(const IConfigReader& reader, const char* path, NumericConstraint<T> constraint) {
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

// template<typename T> requires std::convertible_to<T, std::string_view>
inline SchemaValidator& SchemaValidator::check_string(const IConfigReader& reader, const char* path, StringConstraint constraint) {
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


}