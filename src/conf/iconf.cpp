#include <conf/iconf.hpp>

#include <format>
#include <iterator>

namespace conf {

ValidationResult::ValidationResult(ValidationResult&& other) noexcept {
    this->errors = std::move(other.errors);
}

ValidationResult& ValidationResult::operator = (ValidationResult&& other) noexcept {
    if(this != &other) {
        this->errors = std::move(other.errors);
    }
    return *this;
}

void ValidationResult::append(std::string path, std::string message) noexcept {
    this->errors.push_back(ValidationData{path, message});
}

bool ValidationResult::is_valid() const noexcept {
    return this->errors.empty();
}

std::string ValidationResult::get_log() const noexcept {
    std::string buffer;
    for(const ValidationData& dt : this->errors) {
        std::format_to(std::back_inserter(buffer), "[{}] {}\n", dt.path, dt.message);
    }
    return buffer;
}

std::vector<ValidationData> ValidationResult::get_raw_log() const noexcept {
    return this->errors;
}


// SchemaValidator

SchemaValidator::SchemaValidator(SchemaValidator&& other) noexcept {
    this->_res = std::move(other._res);
}

SchemaValidator& SchemaValidator::operator = (SchemaValidator&& other) noexcept {
    if(this != &other) {
        this->_res = std::move(other._res);
    }
    return *this;
}


ValidationResult SchemaValidator::release() {
    return std::move(this->_res);
}

}