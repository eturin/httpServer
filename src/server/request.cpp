#include "request.hpp"
#include "spdlog/spdlog.h"

namespace http {
    namespace server3 {
        request::request() {}

        bool request::save(pqxx::connection * conn, Context & cont)  const{
            pqxx::work W(*conn);
            if (!conn->is_open() && !cont.prepare(conn))
                return false;
            // URL,Метод,ТипСообщения,Размер,data
            std::size_t pos = uri.find('?');
            pos = pos == std::string::npos ? uri.size() : pos;
            std::string type =uri.substr(0,pos);

            pqxx::result r;

            if (body_size) {
                std::string a(reinterpret_cast<const char*>(&body_size),reinterpret_cast<const char*>(&body_size)+4),
                            b(body.cbegin(),body.cend());
                r = W.exec_prepared("dml_queue_post",type,method,end_point,body_size,a,b);
            } else {
                r = W.exec_prepared("dml_queue_get",type,method,end_point);
            }
            if (r.affected_rows() != 1)
                return false;

            ref = pqxx::to_string(r[0]["Ссылка"]);
            unsigned n = 0;
            for (auto &e : headers)
                pqxx::result r = W.exec_prepared("dml_headers",ref,n,++n,e.name,e.value);

            std::string name, val;
            bool is_val=false;
            n = 0;

            if (pos != uri.size()) {
                for (const char c : uri.substr(pos + 1, uri.size() - pos - 1)) {
                    if (c == '=')
                        is_val = true;
                    else if (c == '&') {
                        pqxx::result r = W.exec_prepared("dml_params", ref, n, ++n, name, val);
                        name = val = "";
                        is_val = false;
                    } else if (is_val)
                        val.push_back(c);
                    else
                        name.push_back(c);
                }
                if (name != "")
                    pqxx::result r = W.exec_prepared("dml_params", ref, n, ++n, name, val);
            }

            W.commit();
            spdlog::info("Запрос сохранен в ПТС НСИ");

            return true;
        }
    }
}
