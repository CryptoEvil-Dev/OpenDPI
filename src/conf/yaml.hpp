#include <conf/iconf.hpp>
#include <ryml.hpp>


namespace conf {

class YAML : public IConf {
public:
    YAML(const std::string& path);
    YAML(const YAML&) = default;
    YAML(YAML&&) noexcept;

    YAML& operator = (const YAML&) = default;
    YAML& operator = (YAML&&) noexcept;

    ~YAML();

    void reload(const std::string& path) override;
    net::SocketAddr dpi_addr() const noexcept override; // required
    net::SocketAddr backend_addr() const noexcept override; // required

    std::string pubkey() const noexcept override; // optional
    std::string privkey() const noexcept override; // optional
    std::string chainkey() const noexcept override; // optional
    bool renewal() const noexcept override; // optional

    std::chrono::seconds ban_duration() const noexcept override; // required
    uint64_t max_rps() const noexcept override; // required
    std::chrono::seconds max_session_duration() const noexcept override; // required
    uint64_t max_packet_size() const noexcept override; // required

private:
    ValidationResult validate(const std::string& config) override;
    ryml::Tree yaml;
};

}