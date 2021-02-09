#include <unistd.h>

#include <iostream>
#include <boost/lexical_cast.hpp>
#include <wait.h>

#include "src/server/server.hpp"
#include "src/server/context.hpp"

#include "src/server/common.hpp"

#define ENV(name,val) std::getenv(name) ? std::getenv(name): val
#define BUF_SIZE 50

int main(int argc, char* argv[]) {
    const char *db_host      = ENV("DB_HOST"     ,"192.168.52.80"),
               *db_port      = ENV("DB_PORT"     ,"5432"         ),
               *db_name      = ENV("DB_NAME"     ,"NSI"          ),
               *db_user      = ENV("DB_USER"     ,"user1c"       ),
               *db_user_pass = ENV("DB_USER_PASS","sGLaVj4PUw"   );
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

    std::shared_ptr<spdlog::logger> syslog_logger = spdlog::syslog_logger_mt("syslog", "api-cpp", LOG_PID | LOG_CONS);

    try {
        if (argc != 5) {
            std::cerr << "Использование: http_server <address> <port> <threads> <doc_root>\n"
                      << "  Для IPv4:\n"
                      << "    "<<argv[0]<<" 0.0.0.0 80 1 .\n"
                      << "  Для IPv6:\n"
                      << "    "<<argv[0]<<" 0::0 80 1 .\n";
            return 1;
        }


        syslog_logger->info("Start.");
        syslog_logger->info("Настройки: {0}, {1}, {2}, {3}, {4}", DB_HOST, DB_PORT, DB_NAME, DB_USER, DB_USER_PASS);
        // инициализация приложения
        int fd_in[2], fd_out[2], fd_err[2], pid;
        if (-1==pipe(fd_in) || -1==pipe(fd_out) || -1==pipe(fd_err)) {
            int err=errno;
            syslog_logger->error("Не удалось сделать pipe: {}", err);
            exit(1);
        }else if ((pid=fork())==-1) {
            int err=errno;
            syslog_logger->error("Не удалось сделать fork: {}", err);
            exit(err);
        }

        if(!pid){
            // дочерний процесс
            if (-1==close(fd_in[1])
               || -1==close(fd_out[0])
               || -1==close(fd_err[0])) {
                int err=errno;
                syslog_logger->error("Не удалось закрыть дескриптор: {}", err);
                exit(err);
            }
            if (-1==dup2(fd_in[0],STDIN_FILENO)
                || -1==dup2(fd_out[1],STDOUT_FILENO)
                || -1==dup2(fd_err[1],STDERR_FILENO)) {
                int err=errno;
                spdlog::error("Не удалось сделать dup2: {}", err);
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
                syslog_logger->error("Не удалось заменить исполняемый код (execvpe): {}", err);
                exit(err);
            }
        } else if (-1==close(fd_in[0])
                   || -1==close(fd_out[1])
                   || -1==close(fd_err[1])) {
            int err=errno;
            syslog_logger->error("Не удалось закрыть дескриптор: {}", err);
            exit(err);
        }

        char buf[BUF_SIZE+1]={0};
        std::string log;
        while (0<read(fd_out[0], buf, BUF_SIZE)) log.append(buf);
        syslog_logger->info(log);
        {
            log.clear();
            bool is_ok=true;
            while (0<read(fd_err[0], buf, BUF_SIZE)) {
                log.append(buf);
                is_ok = false;
            }
            if (!is_ok) {
                syslog_logger->info(log);
                exit(-1);
            }
        }

        if(-1==close(fd_in[1])
          || -1==close(fd_out[0])
          || -1==close(fd_err[0])) {
            int err=errno;
            syslog_logger->error("Не удалось закрыть дескриптор: {}", err);
            exit(err);
        }
        int  status;
        if (!waitpid(pid,&status,WNOHANG)) kill(pid,9);

        // инициализация сервера
        std::size_t num_threads = boost::lexical_cast<std::size_t>(argv[3]);
        http::server3::Context cont(syslog_logger);
        if (cont.make_pool(num_threads, connection_str)) { // инициализация контекста
            syslog_logger->info("Параметры запуска сервера.... {0} {1} {2} {3}",argv[1],argv[2],num_threads,argv[4]);
            http::server3::server s(argv[1], argv[2], argv[4], num_threads, cont);
            // запуск
            s.run();
        } else {
            syslog_logger->error("Ошибка инициализации сервера");
        }
    } catch (std::exception& e) {
        syslog_logger->error("Ошибка: {}", e.what());
    }

    return 0;
}