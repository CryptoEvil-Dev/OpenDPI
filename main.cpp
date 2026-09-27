#include <iostream>
#include <net/addrs.hpp>
#include <net/tcp.hpp>
#include <thread>
#include <cstring>

struct conf_s {
    net::SocketAddr dpi;
    net::SocketAddr backend;
};

struct conf_s parse_option(int argc, char **argv) {
    struct conf_s tmp {};

    if (argc == 1 || argc == 2 || argc == 4) {
        std::cerr << "usage: " << argv[0] << " --dpi <IP:port> --backend <IP:port> is required" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    if (argc == 3) {
        if (std::strcmp(argv[1], "--dpi") == 0) std::cerr << "Argument `--backend <IP:port>` is required" << std::endl;
        else if (std::strcmp(argv[1], "--backend") == 0) std::cerr << "Argument `--dpi <IP:port>` is required" << std::endl;
        else std::cerr << "Argument `" << argv[1] << "` is unexpected" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    if (argc == 5) {
        if (std::strcmp(argv[1], "--dpi") == 0 && std::strcmp(argv[3], "--backend") == 0) {
            tmp.dpi = net::SocketAddr(argv[2]);
            tmp.backend = net::SocketAddr(argv[4]);
        } else if (std::strcmp(argv[1], "--backend") == 0 && std::strcmp(argv[3], "--dpi") == 0) {
            tmp.dpi = net::SocketAddr(argv[4]);
            tmp.backend = net::SocketAddr(argv[2]);
        } else {
            std::cerr << "usage: " << argv[0] << " --dpi <IP:port> --backend <IP:port> is required" << std::endl;
            exit(EXIT_FAILURE);
        }
    } else if (argc > 5) {
        std::cerr << "Too many arguments" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    return tmp;
}

int main(int argc, char **argv) {
    struct conf_s opts = parse_option(argc, argv);

    std::cout << "DPI: " << opts.dpi << std::endl;
    std::cout << "BCK: " << opts.backend << std::endl;

    auto listener = net::TcpListener::bind(opts.dpi);
    
    while(true) {
        auto conn = listener.accept();
        
        std::string buffer;
        std::string lbf;
        
        while(true) {
            lbf.clear();
            lbf.resize(1500);
            ssize_t readed = conn.read(lbf);
            
            if (readed <= 0) break;
            
            lbf.resize(static_cast<size_t>(readed));
            buffer.append(lbf);

            if (buffer.find("\r\n\r\n") != std::string::npos) {
                break;
            }
        }

        std::cout << "Получен запрос:\n" << buffer << std::endl;

        if (buffer.find("GET / HTTP/1.1") != std::string::npos) {
            std::string response = 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html; charset=UTF-8\r\n"
                "Content-Length: 111\r\n"
                "Server: OpenDPI\r\n"
                "Connection: close\r\n"
                "\r\n"
                "<!DOCTYPE html>\r\n"
                "<html>\r\n"
                "<head><title>OpenDPI WebUI</title></head>\r\n"
                "<body><h1>Hello World!</h1></body>\r\n"
                "</html>";
                
            conn.write(response);
        }

        conn.close();
        buffer.clear();
    }

    return 0;
}