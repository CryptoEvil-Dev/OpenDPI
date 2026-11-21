#include <iostream>

#include <thread>
#include <conf/yaml.hpp>

#include <net/addrs.hpp>
#include <net/tcp.hpp>

#include <event/events.hpp>


class Bootstrap {
public:
    Bootstrap(int argc, char **argv) {
        if(argc != 1) {
            for(int i = 0; i < argc; ++i) {
                if(argv[i] == "--config" || argv[i] == "-c") {
                    this->_conf_path = argv[i+1];
                    break;
                }
            }
            if(!this->_conf_path.empty()) {
                std::cerr << "Configuration file is not found!" << std::endl;
                exit(1);
            }
        } else this->_conf_path = "/var/log/OpenDPI/default.yaml";
        try {
            this->_reader = std::make_unique<conf::RyamlConfigReader>(this->_conf_path);
        } catch(const std::system_error& e) {
            if(e.code() == std::errc::no_such_file_or_directory)
                std::cerr << this->_conf_path << " is not exists" << std::endl;
            if(e.code() == std::errc::permission_denied)
                std::cerr << this->_conf_path << " is permission denied" << std::endl;
            exit(1);
        } catch (const std::runtime_error& e) {
            std::cerr << this->_conf_path << " is undefined error: " << e.what() << std::endl;
            exit(2);
        }
    }
    Bootstrap(const Bootstrap&) = delete;
    Bootstrap(Bootstrap&&) = delete;
    ~Bootstrap() = default;

    conf::IConfigReader* get_reader() {
        return this->_reader.get();
    };

private:
    std::unique_ptr<conf::IConfigReader> _reader;
    std::string _conf_path;
};

int main([[maybe_unused]]int argc, [[maybe_unused]]char **argv) {
    Bootstrap bts(argc, argv);
    // conf::RyamlConfigReader reader = bts.get_reader();


    return 0;
}