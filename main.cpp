#include <iostream>
#include <net/addrs.hpp>
#include <net/tcp.hpp>

// #include <conf/iconf.hpp>
#include <conf/yaml.hpp>


int main(int argc, char **argv) {
    conf::RyamlConfigReader reader("config.yaml");
    conf::SchemaValidator validator;
    validator.check_anytype<net::SocketAddr>(reader, "system.dpi")
             .check_anytype<net::SocketAddr>(reader, "system.backend")
             .check_numeric(reader, "system.mtu", conf::NumericConstraint<int>{.min=100, .max=1500});

    conf::ValidationResult vr = validator.release();
    std::cout << vr.get_log() << std::endl;
    std::cout << (vr.is_valid() ? "is valid" : "invalid") << std::endl;


    return 0;
}