#include <conf/yaml.hpp>
#include <ryml.hpp>
#include <fstream>
#include <filesystem>

namespace conf {

struct RyamlConfigReader::Impl {
    ryml::Tree tree;
};

RyamlConfigReader::RyamlConfigReader(const std::string& path) : _impl(std::make_unique<Impl>()) {
    {
        std::filesystem::path _file(path);
        if(!std::filesystem::exists(_file)) {
            throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory));
        }
    }

    std::ifstream file(path);
    if(!file.is_open()) {
        if(errno == EACCES || errno == EPERM)
            throw std::system_error(std::make_error_code(std::errc::permission_denied));
        else {
            std::string err_buf;
            std::format_to(std::back_inserter(err_buf), "Ошибка при открытии файла: {}", errno);
            throw std::runtime_error(err_buf);
        }
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    try {
        this->_impl->tree = ryml::parse_in_arena(ryml::csubstr{content.data(), content.size()});
    } catch(const std::exception&) {
        throw std::runtime_error("syntax error");
    }
}

RyamlConfigReader::RyamlConfigReader(RyamlConfigReader&& other) noexcept {
    this->_impl->tree = std::move(other._impl->tree);
}

RyamlConfigReader::~RyamlConfigReader() = default;

RyamlConfigReader& RyamlConfigReader::operator = (RyamlConfigReader&& other) noexcept {
    if(this != &other) {
        this->_impl->tree = std::move(other._impl->tree);
    }
    return *this;
}

std::optional<std::string_view> RyamlConfigReader::get_value(std::string_view path) const {
    ryml::ConstNodeRef root = this->_impl->tree.rootref();

    auto move = [&root](std::string_view _path) -> void {
        if(root.invalid()) return;
        root = root.find_child(c4::csubstr{_path.data(), _path.size()});
    };

    size_t start = 0;
    while (start <= path.size())
    {
        size_t pos = path.find('.', start);
        if(pos == std::string_view::npos) {
            move(path.substr(start));
            break;
        }
        move(path.substr(start, pos - start));
        start = pos + 1;
    }
    
    if(root.invalid()) return std::nullopt;
    if(!root.has_val()) return std::nullopt;
    c4::csubstr raw = root.val();
    return std::string_view(raw.str, raw.len);
}


}