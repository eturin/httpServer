#include <unistd.h>
#define ENV(name,val) std::getenv(name) ? std::getenv(name): val
#include "request_handler.hpp"
#include <fstream>
#include <string>

#include <boost/thread.hpp>
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"
#include "connection.hpp"

#include <iostream>
#include "spdlog/spdlog.h"

namespace http {
    namespace server3 {

        request_handler::request_handler(const std::string& doc_root, Context &context) : doc_root_(doc_root),
                                                                                          context(context) {}

        bool request_handler::handle_request(const request& req,
                                             reply& rep,
                                             connection * client) {
            // декодируем url-строку
            std::string request_path;
            if (!url_decode(req.uri, request_path)) {
                rep = reply::stock_reply(reply::bad_request);
                context.syslog_logger->warn("Не удалось декодировать uri: {}", req.uri);
                return true;
            }

            // запрашиваемый путь должен быть абсолютный и не содержать ".."
            if (request_path.empty()
                || request_path[0] != '/'
                || request_path.find("..") != std::string::npos) {
                rep = reply::stock_reply(reply::bad_request);
                context.syslog_logger->warn("запрашиваемый путь должен быть абсолютный и не содержать \"..\": {}", request_path);
                return true;
            }

            // если запрашивается корнень, то отдавать будем index.html
            if (request_path[request_path.size() - 1] == '/')
                request_path += "index.html";

            if (request_path.rfind("/sync/") == 0
                || request_path.rfind("/async/") == 0) {

                //определяем обработчик
                pqxx::connection *conn = context.get_conn();
                if (!conn->is_open() && !context.prepare(conn)) {
                    context.syslog_logger->error("Не настроено соединение с СУБД");
                    return true;
                }
                pqxx::work W(*conn);
                pqxx::result r = W.exec_prepared("is_cpp",req.end_point);
                W.commit();
                if (r.affected_rows()) {
                    client->sync = r[0][0].c_str()[0] =='t';


                    //  сохраняем
                    if(!client->sync && !req.save(context.get_conn(), context)) {
                        rep = reply::stock_reply(reply::internal_server_error);
                        context.syslog_logger->error("Запрс не сохранен в ПТС НСИ");
                        return true;
                    };

                    //запуск обработчика
                    if (-1==pipe(client->fd_in) || -1==pipe(client->fd_out) || -1==pipe(client->fd_err)) {
                        int err=errno;
                        context.syslog_logger->error("Не удалось сделать pipe: {}", err);
                        return true;
                    }else if ((client->pid=fork())==-1) {
                        int err=errno;
                        context.syslog_logger->error("Не удалось сделать fork: {}", err);
                        return true;
                    }

                    if(!client->pid){
                        // дочерний процесс
                        if (-1==close(client->fd_in[1])
                            || -1==close(client->fd_out[0])
                            || -1==close(client->fd_err[0])) {
                            int err=errno;
                            context.syslog_logger->error("Не удалось закрыть дескриптор в дочернем процессе обработчика запроса: {}", err);
                            exit(1);
                        }
                        if (-1==dup2(client->fd_in[0],STDIN_FILENO)
                            || -1==dup2(client->fd_out[1],STDOUT_FILENO)
                            || -1==dup2(client->fd_err[1],STDERR_FILENO)) {
                            int err=errno;
                            context.syslog_logger->error("Не удалось сделать dup2: {}", err);
                            exit(1);
                        }
                        // задаем переменные окружения загружаемой программы
                        const char *db_host      = ENV("DB_HOST"     ,"192.168.52.80"),
                                   *db_port      = ENV("DB_PORT"     ,"5432"         ),
                                   *db_name      = ENV("DB_NAME"     ,"NSI"          ),
                                   *db_user      = ENV("DB_USER"     ,"user1c"       ),
                                   *db_user_pass = ENV("DB_USER_PASS","sGLaVj4PUw"   );
                        std::vector<std::string> env;
                        for (const auto &x : req.headers) env.push_back (x.name + "=" + x.value);
                        for (const auto &x : req.params)  env.push_back (x.name + "=" + x.value);
                        env.push_back ("uri=" + req.uri);
                        env.push_back (std::string("DB_HOST=")+db_host);
                        env.push_back (std::string("DB_PORT=")+db_port);
                        env.push_back (std::string("DB_NAME=")+db_name);
                        env.push_back (std::string("DB_USER=")+db_user);
                        env.push_back (std::string("DB_USER_PASS=")+db_user_pass);

                        std::vector<const char*> _env;
                        for (const auto &x : env)  _env.push_back(x.c_str());
                        std::string path("handlers/");

                        path.append(req.end_point);
                        char *const _argv[] = {(char* const)path.c_str(), nullptr};
                        if (-1 == execvpe(_argv[0], _argv, (char *const *)&_env[0])) {
                            int err=errno;
                            context.syslog_logger->error("Не удалось заменить исполняемый код (execvpe): {}",err);
                            exit(1);
                        }
                    } else if (-1==close(client->fd_in[0])
                               || -1==close(client->fd_out[1])
                               || -1==close(client->fd_err[1])) {
                        int err=errno;
                        context.syslog_logger->error("Не удалось закрыть дескриптор: {}", err);
                        return  true;
                    }
                    //передаем body
                    return false;
                } else {
                    //  сохраняем
                    if(!req.save(context.get_conn(), context)) {
                        rep = reply::stock_reply(reply::internal_server_error);
                        context.syslog_logger->error("Запрс не сохранен в ПТС НСИ");
                        return true;
                    };
                    //ответ при отсутствие обработчика
                    rep.status = reply::not_implemented;

                    rep.content.append("Не назначен обработчик");

                    rep.headers.resize(2);
                    rep.headers[0].name = "Content-Length";
                    rep.headers[0].value = std::to_string(rep.content.size());
                    rep.headers[1].name = "Content-Type";
                    rep.headers[1].value = "text";
                }
            } else {
                // определение расширения запрошенного файла
                std::size_t last_slash_pos = request_path.find_last_of('/');
                std::size_t last_dot_pos = request_path.find_last_of('.');
                std::string extension;
                if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
                    extension = request_path.substr(last_dot_pos + 1);


                // открываем файл, который запрашивает клиент
                std::string full_path = doc_root_ + request_path;
                std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
                if (!is) {
                    rep = reply::stock_reply(reply::not_found);
                    return true;
                }

                // формируем ответ клиенту с содержанием файла
                rep.status = reply::ok;

                char buf[512];
                while (is.read(buf, sizeof(buf)).gcount() > 0) rep.content.append(buf, is.gcount());

                rep.headers.resize(2);
                rep.headers[0].name = "Content-Length";
                rep.headers[0].value = std::to_string(rep.content.size());
                rep.headers[1].name = "Content-Type";
                rep.headers[1].value = mime_types::extension_to_type(extension);
            }
            return true;
        }

        bool request_handler::url_decode(const std::string& in, std::string& out) {
            out.clear();
            out.reserve(in.size());
            for (std::size_t i = 0; i < in.size(); ++i) {
                if (in[i] == '%') {
                    if (i + 3 <= in.size()) {
                        int value = 0;
                        std::istringstream is(in.substr(i + 1, 2));
                        if (is >> std::hex >> value) {
                            out += static_cast<char>(value);
                            i += 2;
                        } else
                            return false;
                    } else
                        return false;
                } else if (in[i] == '+')
                    out += ' ';
                else
                    out += in[i];
            }

            return true;
        }
    } // namespace server3
} // namespace http