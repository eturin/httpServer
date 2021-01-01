#include "context.hpp"

namespace http {
    namespace server3 {
        Context::Context() : uuid_num(),conns() {}

        bool Context::prepare(pqxx::connection *conn) {
            conn->activate();
            if (conn->is_open()) {
                conn->prepare("dml_queue", R"(
                --запрос
                insert into queue(method,end_point,uri,body,created)
                values           (  $1  ,   $2    , $3, $4 ,now()  )
                returning id;
                )");
                conn->prepare("dml_headers", R"(
                --заголовки запроса
                insert into queue_headers(queue_id,name,value)
                values                   (   $1   , $2 ,  $3 )
                )");

                return true;
            } else return false;
        }

        bool Context::make_pool(std::size_t cnt, const std::string & connection_string){
            try {
                for (std::size_t i = 0; i<cnt; ++i) {
                    conns.push_back(new pqxx::connection(connection_string));
                    if (!conns[i]->is_open()) return false;
                    else prepare(conns[i]);
                }
            } catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
                return false;
            }

            return true;
        }

        pqxx::connection* Context::get_conn() {
            auto it = uuid_num.find(boost::lexical_cast<std::string>(boost::this_thread::get_id()));
            if ( it != uuid_num.end())
                return conns[it->second];

            return nullptr;
        }

        void Context::add_uuid_num(boost::thread::id uuid,char num) {
            uuid_num[boost::lexical_cast<std::string>(uuid)] = num;
        }

        Context::~Context() {
            for (pqxx::connection * e : conns) {
                if (e->is_open()) e->disconnect();
                delete e;
            }
        }
    }
}