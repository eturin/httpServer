#include "context.hpp"

namespace http {
    namespace server3 {
        Context::Context() : uuid_num(),conns() {}

        bool Context::make_pool(std::size_t cnt, const std::string & connection_string){
            try {
                for (std::size_t i = 0; i<cnt; ++i) {
                    conns.push_back(new pqxx::connection(connection_string));
                    if (!conns[0]->is_open()) return false;
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
            for (pqxx::connection * e : conns)
                delete e;
        }
    }
}