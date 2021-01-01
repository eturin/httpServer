#ifndef HTTP_SERVER3_REQUEST_HPP
#define HTTP_SERVER3_REQUEST_HPP

#include <string>
#include <vector>
#include <pqxx/pqxx>
#include "header.hpp"

namespace http {
    namespace server3 {

        // запрос полученный от клиента.
        struct request {
            bool save(pqxx::connection * conn) const;
            std::string method;
            std::string uri;
            int http_version_major;
            int http_version_minor;
            std::vector<header> headers;
            std::size_t body_size;
            std::vector<char> body;
        private:
            std::size_t id;

        };
    } // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_HPP