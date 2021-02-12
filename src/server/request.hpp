#ifndef HTTP_SERVER3_REQUEST_HPP
#define HTTP_SERVER3_REQUEST_HPP

#include <string>
#include <vector>
#include <pqxx/pqxx>
#include "header.hpp"
#include "context.hpp"
#include "common.hpp"

namespace http {
    namespace server3 {

        // запрос полученный от клиента.
        struct request {
            request();
            bool save(pqxx::connection * conn, Context & cont) const;
            std::string method;
            std::string uri;
            std::string end_point;
            int http_version_major;
            int http_version_minor;
            std::vector<header> headers;
            std::vector<header> params;
            std::size_t body_size;
            std::vector<char> body;
            mutable std::string ref;

        };
    } // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_HPP