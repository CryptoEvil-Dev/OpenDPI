#include <conf/yaml.hpp>
#include <iostream>
#include <fstream>
#include <sstream>


namespace conf {

YAML::YAML(const std::string& path) {
    std::ifstream file(path);
    if(!file.is_open()) {
        throw std::errc::no_such_file_or_directory;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    this->yaml = ryml::parse_in_arena(ryml::to_csubstr(content));
}

YAML::YAML(YAML&& other) noexcept {
    this->yaml = std::move(other.yaml);
}

YAML& YAML::operator = (YAML&& other) noexcept {
    if(this != &other) {
        this->yaml = std::move(other.yaml);
    }
    return *this;
}

void YAML::reload(const std::string& path) {
    std::ifstream file(path);
    if(!file.is_open()) {
        throw std::errc::no_such_file_or_directory;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    

    this->yaml.clear();
    ryml::parse_in_arena(ryml::to_csubstr(content), this->yaml);
}





namespace {
    ryml::NodeRef safe_find(ryml::NodeRef root, std::initializer_list<const char*> path) {
        ryml::NodeRef current = root;
        for(const char* key : path) {
            if(current.invalid()) return current;
            current = current.find_child(key);
        }

        return current;
    }
}


// In Progress
ValidationResult YAML::validate(const std::string& config) {
    ValidationResult res;
    ryml::Tree tree;
    try {
        tree = ryml::parse_in_arena(ryml::to_csubstr(config));
    } catch(const std::exception& ex) {
        res.append("global", "Syntax Error");
        return res;
    }

    ryml::NodeRef root = tree.rootref();

    {
        ryml::NodeRef ref = safe_find(root, {"system", "dpi"});
        if(!ref.invalid() && ref.has_val() && !ref.empty()) {
            try {
                c4::csubstr raw = ref.val();
                std::string_view sv(raw.str, raw.len);
                net::SocketAddr{sv};
            } catch(...) {
                res.append("system.dpi", "invalid value");
            }
        } else {
            res.append("system.dpi", "is missing, empty or invalid");
        }
    }

    {
        ryml::NodeRef ref = safe_find(root, {"system", "backend"});
        if(!ref.invalid() && ref.has_val() && !ref.empty()) {
            try {
                c4::csubstr raw = ref.val();
                std::string_view sv(raw.str, raw.len);
                net::SocketAddr{sv};
            } catch(...) {
                res.append("system.backend", "invalid value");
            }
        } else {
            res.append("system.backend", "is missing, empty or invalid");
        }
    }

    {
        ryml::NodeRef ref = safe_find(root, {"filter", "anonymous", "rps"});
        if(!ref.invalid() && ref.has_val() && !ref.empty()) {
            c4::csubstr raw = ref.val();
            std::string_view sv(raw.str, raw.len);
            uint64_t value = 0;

            auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
            if(ec != std::errc{}) {
                res.append("filter.anonymous.rps", "invalid value");
            } else {
                res.append("filter.anonymous.rps", "is missing, empty or invalid");
            }
        }
    }

    

    return res;
}

}