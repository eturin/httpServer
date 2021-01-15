#include <unistd.h>

#include <iostream>
#include <boost/lexical_cast.hpp>

#include "src/server/server.hpp"
#include "src/server/context.hpp"

#define ENV(name,val) std::getenv(name) ? std::getenv(name): val
#define BUF_SIZE 50

int main(int argc, char* argv[]) {
    const char *db_host      = ENV("DB_HOST","127.0.0.1"),
               *db_port      = ENV("DB_PORT","5432"),
               *db_name      = ENV("DB_NAME","mdm"),
               *db_user      = ENV("DB_USER","postgres"),
               *db_user_pass = ENV("DB_USER_PASS","123");
    std::string connection_str =std::string(R"(
            dbname   = )")+ db_name      + R"(
            user     = )" + db_user      + R"(
            password = )" + db_user_pass + R"(
            hostaddr = )" + db_host      + R"(
            port     = )" + db_port      + R"(
    )";
    std::string DB_HOST=std::string("DB_HOST=")+db_host,
                DB_PORT=std::string("DB_PORT=")+db_port,
                DB_NAME=std::string("DB_NAME=")+db_name,
                DB_USER=std::string("DB_USER=")+db_user,
                DB_USER_PASS=std::string("DB_USER_PASS=")+db_user_pass;
    try {
        if (argc != 5) {
            std::cerr << "Использование: http_server <address> <port> <threads> <doc_root>\n";
            std::cerr << "  Для IPv4:\n";
            std::cerr << "    "<<argv[0]<<" 0.0.0.0 80 1 .\n";
            std::cerr << "  Для IPv6:\n";
            std::cerr << "    "<<argv[0]<<" 0::0 80 1 .\n";
            return 1;
        }
        // инициализация приложения
        int fd_in[2], fd_out[2], fd_err[2], pid;
        if (-1==pipe(fd_in) || -1==pipe(fd_out) || -1==pipe(fd_err)) {
            int err=errno;
            perror("Не удалось сделать pipe");
            exit(err);
        }else if ((pid=fork())==-1) {
            int err=errno;
            perror("Не удалось сделать fork");
            exit(err);
        }

        if(!pid){
            // дочерний процесс
            if (-1==close(fd_in[1])
               || -1==close(fd_out[0])
               || -1==close(fd_err[0])) {
                int err=errno;
                perror("Не удалось закрыть дескриптор");
                exit(err);
            }
            if (-1==dup2(fd_in[0],STDIN_FILENO)
                || -1==dup2(fd_out[1],STDOUT_FILENO)
                || -1==dup2(fd_err[1],STDERR_FILENO)) {
                int err=errno;
                perror("Не удалось сделать dup2");
                exit(err);
            }
            // задаем переменные окружения загружаемой программы
            char *const _env[] ={(char* const)(DB_HOST.c_str()),
                                 (char* const)(DB_PORT.c_str()),
                                 (char* const)(DB_NAME.c_str()),
                                 (char* const)(DB_USER.c_str()),
                                 (char* const)(DB_USER_PASS.c_str()),
                                 nullptr};
            char *const _argv[] = {(char* const)"handlers/init", nullptr};
            if(-1 == execvpe(_argv[0], _argv, _env)) {
                int err=errno;
                perror("Не удалось заменить исполняемый код (execvpe)");
                exit(err);
            }
        } else if (-1==close(fd_in[0])
                   || -1==close(fd_out[1])
                   || -1==close(fd_err[1])) {
            int err=errno;
            perror("Не удалось закрыть дескриптор");
            exit(err);
        }

        char buf[BUF_SIZE+1]={0};
        while (0<read(fd_out[0], buf, BUF_SIZE))
            std::cout << buf;
        {
            bool is_ok=true;
            while (0<read(fd_err[0], buf, BUF_SIZE)) {
                std::cerr << buf;
                is_ok = false;
            }
            if (!is_ok) {
                exit(-1);
            }
        }

        if(-1==close(fd_in[1])
          || -1==close(fd_out[0])
          || -1==close(fd_err[0])) {
            int err=errno;
            perror("Не удалось закрыть дескриптор");
            exit(err);
        }

        // инициализация сервера
        std::size_t num_threads = boost::lexical_cast<std::size_t>(argv[3]);
        http::server3::Context cont;
        if (cont.make_pool(num_threads, connection_str)) { // инициализация контекста
            std::cout << "Starting server... " << argv[1] << " " << argv[2] << " " << num_threads << " " << argv[4]
                      << '\n';
            http::server3::server s(argv[1], argv[2], argv[4], num_threads, cont);
            // запуск
            s.run();
        } else {
            std::cerr << "Ошибка инициализации\n";
        }
    } catch (std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << "\n";
    }

    return 0;
}