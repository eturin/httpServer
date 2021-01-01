#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <pqxx/pqxx>
#include <boost/thread/thread.hpp>
#include <boost/lexical_cast.hpp>

namespace http {
    namespace server3 {
        struct Context {
        public:
             Context();
             bool make_pool(std::size_t cnt, const std::string & connection_string);
             void add_uuid_num(boost::thread::id uuid, char num);
             pqxx::connection* get_conn();
             ~Context();
        private:
             std::vector<pqxx::connection*> conns;
             std::unordered_map<std::string, char> uuid_num;
        };
    }
}