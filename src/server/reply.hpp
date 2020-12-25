#ifndef HTTP_SERVER3_REPLY_HPP
#define HTTP_SERVER3_REPLY_HPP

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "header.hpp"

namespace http {
    namespace server3 {

       // структура ответа
        struct reply {
            // статус
            enum status_type {
                ok = 200,
                created = 201,
                accepted = 202,
                no_content = 204,
                multiple_choices = 300,
                moved_permanently = 301,
                moved_temporarily = 302,
                not_modified = 304,
                bad_request = 400,
                unauthorized = 401,
                forbidden = 403,
                not_found = 404,
                internal_server_error = 500,
                not_implemented = 501,
                bad_gateway = 502,
                service_unavailable = 503
            } status;

            // заголовки
            std::vector<header> headers;

            // тело ответа
            std::string content;

            /* Преобразуйте ответ в вектор буферов.
             * Буферы не владеют нижележащими блоками памяти,
             * поэтому объект ответа должен оставаться действительным
             * и не изменяться до завершения операции записи. */
            std::vector<boost::asio::const_buffer> to_buffers();

            // формирование стандартного ответа
            static reply stock_reply(status_type status);
        };
    } // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REPLY_HPP