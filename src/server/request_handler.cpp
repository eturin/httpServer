#include "request_handler.hpp"
#include <fstream>
#include <string>

#include <boost/thread.hpp>
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"

#include <iostream>

namespace http {
    namespace server3 {

        request_handler::request_handler(const std::string& doc_root, Context &context) : doc_root_(doc_root),
                                                                                          context(context) {}

        void request_handler::handle_request(const request& req, reply& rep) {
            // декодируем url-строку
            std::string request_path;
            if (!url_decode(req.uri, request_path)) {
                rep = reply::stock_reply(reply::bad_request);
                return;
            }

            // запрашиваемый путь должен быть абсолютный и не содержать ".."
            if (request_path.empty()
                || request_path[0] != '/'
                || request_path.find("..") != std::string::npos) {
                rep = reply::stock_reply(reply::bad_request);
                return;
            }

            // если запрашивается корнень, то отдавать будем index.html
            if (request_path[request_path.size() - 1] == '/')
                request_path += "index.html";

            if (request_path.rfind("/sync/") == 0
                || request_path.rfind("/async/") == 0) {
                //логируем
                {
                    req.save(context.get_conn());
                    //запрос
                    std::size_t k = request_path.find('/',1);
                    if (k != request_path.npos){
                        std::string end_point = request_path.substr(k+1,request_path.find('?')-k-1);
                        std::cout << req.method << " " << request_path << ' '<< end_point <<'\n';

                        //заголовки
                        for (auto &e : req.headers) {
                            std::cout << '\t' << e.name << " = " << e.value << '\n';
                        }
                    } else {
                        //err
                    }
                }
                //обработчик
                //ответ
                ;
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
                    return;
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