#include <iostream>
#include <boost/lexical_cast.hpp>
#include "src/server/server.hpp"

int main(int argc, char* argv[]) {
    try {
        if (argc != 5) {
            std::cerr << "Использование: http_server <address> <port> <threads> <doc_root>\n";
            std::cerr << "  Для IPv4:\n";
            std::cerr << "    "<<argv[0]<<" 0.0.0.0 80 1 .\n";
            std::cerr << "  Для IPv6:\n";
            std::cerr << "    "<<argv[0]<<" 0::0 80 1 .\n";
            return 1;
        }

        // инициализация сервера
        std::size_t num_threads = boost::lexical_cast<std::size_t>(argv[3]);
        http::server3::server s(argv[1], argv[2], argv[4], num_threads);

        // запуск
        s.run();
    } catch (std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << "\n";
    }

    return 0;
}