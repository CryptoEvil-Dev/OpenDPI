#include <conf/iconf.hpp>
// #include <ryml.hpp>
#include <memory>


namespace conf {

class RyamlConfigReader : public IConfigReader {
public:
    RyamlConfigReader(const std::string& path);
    RyamlConfigReader(const RyamlConfigReader&) = default;
    RyamlConfigReader(RyamlConfigReader&&) noexcept;
    ~RyamlConfigReader() override;

    RyamlConfigReader& operator = (const RyamlConfigReader&) = default;
    RyamlConfigReader& operator = (RyamlConfigReader&&) noexcept;

    std::optional<std::string_view> get_value(std::string_view path) const override;

private:
    // ryml::Tree _tree;
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}