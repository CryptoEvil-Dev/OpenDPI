#include <iostream>
#include <net/addrs.hpp>
#include <net/tcp.hpp>
#include <conf/yaml.hpp>


class MicroCore {
public:
    MicroCore() {
        this->config = std::make_unique<conf::RyamlConfigReader>("config.yaml");
        std::string yaml_path = "system.dpi";
        this->listener = std::make_unique<net::TcpListener>(net::TcpListener::bind(net::SocketAddr(this->config->get_value(yaml_path).value())));
    }
    MicroCore(const MicroCore&) = delete;
    MicroCore(MicroCore&&) = delete;
    MicroCore& operator = (const MicroCore&) = delete;
    MicroCore& operator = (MicroCore&) = delete;


    net::SocketAddr dpi_addr() const {
        std::string path = "system.dpi";
        return net::SocketAddr(this->config->get_value(path).value());
    }
    net::SocketAddr backend_addr() const {
        std::string path = "system.backend";
        return net::SocketAddr(this->config->get_value(path).value());
    };

    void reload() {
        auto new_config = std::make_unique<conf::RyamlConfigReader>("config.yaml");
        auto dpi_str = new_config->get_value("system.dpi").value();
        auto new_addr = net::SocketAddr(dpi_str);

        auto new_listener = net::TcpListener::bind(new_addr);
        auto new_listener_ptr = std::make_unique<net::TcpListener>(std::move(new_listener));

        this->config = std::move(new_config);
        this->listener->close();
        this->listener = std::move(new_listener_ptr);
        // this->config = std::make_unique<conf::RyamlConfigReader>("config.yaml");
        // this->listener->close();
        // std::string yaml_path = "system.dpi";
        // this->listener = std::make_unique<net::TcpListener>(net::TcpListener::bind(net::SocketAddr(this->config->get_value(yaml_path).value())));
    }

    std::unique_ptr<net::TcpStream> accept() {
        return std::make_unique<net::TcpStream>(this->listener->accept());
    }


private:
    std::unique_ptr<conf::IConfigReader> config;
    std::unique_ptr<net::TcpListener> listener;
};




int main() {
    MicroCore _core;
    std::cout << "DPI: " << _core.dpi_addr() << std::endl;
    std::cout << "BCK: " << _core.backend_addr() << std::endl;

    while(true) {
        auto conn = _core.accept();
        std::cout << "conn created: " << conn->descriptor() << std::endl;
        std::string buffer;
        buffer.resize(1500);
        
        ssize_t rd = conn->read(buffer);
        conn->write(buffer);
        conn->close();

        if(rd == -1) continue;
        buffer.resize(rd);
        if(buffer == "reload\n") {
            _core.reload();
            std::cout << "Config reloaded!" << std::endl;
        }
    }
}






// int smain([[maybe_unused]]int argc, [[maybe_unused]]char **argv) {
//     MicroCore _core;
//     std::cout << "DPI: " << _core.dpi_addr() << std::endl;
//     std::cout << "BCK: " << _core.backend_addr() << std::endl;

//     auto listener = net::TcpListener::bind(_core.dpi_addr());
    
//     while(true) {
//         auto conn = listener.accept();
        
//         std::string buffer;
//         std::string lbf;
        
//         while(true) {
//             lbf.clear();
//             lbf.resize(1500);
//             ssize_t readed = conn.read(lbf);
            
//             if (readed <= 0) break;
            
//             lbf.resize(static_cast<size_t>(readed));
//             buffer.append(lbf);

//             if (buffer.find("\r\n\r\n") != std::string::npos) {
//                 break;
//             }
//         }

//         std::cout << "Получен запрос:\n" << buffer << std::endl;

//         if (buffer.find("GET / HTTP/1.1") != std::string::npos) {
//             std::string response = 
//                 "HTTP/1.1 200 OK\r\n"
//                 "Content-Type: text/html; charset=UTF-8\r\n"
//                 "Content-Length: 111\r\n"
//                 "Server: OpenDPI\r\n"
//                 "Connection: close\r\n"
//                 "\r\n"
//                 "<!DOCTYPE html>\r\n"
//                 "<html>\r\n"
//                 "<head><title>OpenDPI WebUI</title></head>\r\n"
//                 "<body><h1>Hello World!</h1></body>\r\n"
//                 "</html>";
                
//             conn.write(response);
//         } else if(buffer.find("GET /reload HTTP/1.1") != std::string::npos) {
//             std::string response = 
//                 "HTTP/1.1 200 OK\r\n"
//                 "Content-Type: text/html; charset=UTF-8\r\n"
//                 "Content-Length: 111\r\n"
//                 "Server: OpenDPI\r\n"
//                 "Connection: close\r\n"
//                 "\r\n"
//                 "<!DOCTYPE html>\r\n"
//                 "<html>\r\n"
//                 "<head><title>OpenDPI WebUI</title></head>\r\n"
//                 "<body><h1>Configuration reloaded!</h1></body>\r\n"
//                 "</html>";
                
//             conn.write(response);
//             _core.reload("config.yaml");
//             std::cout << "DPI: " << _core.dpi_addr() << std::endl;
//             std::cout << "BCK: " << _core.backend_addr() << std::endl;
//         } else {
//             std::string response = 
//                 "HTTP/1.1 404 Not Found\r\n"
//                 "Content-Type: text/html; charset=UTF-8\r\n"
//                 "Content-Length: 112\r\n"
//                 "Server: OpenDPI\r\n"
//                 "Connection: close\r\n"
//                 "\r\n"
//                 "<!DOCTYPE html>\r\n"
//                 "<html>\r\n"
//                 "<head><title>OpenDPI WebUI</title></head>\r\n"
//                 "<body><h1>404 Not Found</h1></body>\r\n"
//                 "</html>";
            
//             conn.write(response);
//         }

//         conn.close();
//         buffer.clear();
//     }

//     return 0;
// }