#ifndef HTTP_SERVER3_REQUEST_HANDLER_HPP
#define HTTP_SERVER3_REQUEST_HANDLER_HPP

#include <string>
#include <boost/noncopyable.hpp>

namespace http {
    namespace server3 {
        struct reply;
        struct request;

        // Общая обработка всех входящих запросов
        class request_handler : private boost::noncopyable {
        public:
            explicit request_handler(const std::string& doc_root);

            // обработка запроса и формирование ответа
            void handle_request(const request& req, reply& rep);
        private:
            // корневой каталог сервера
            std::string doc_root_;

            // декодирует строку URL и возвращает false, если строка не верная
            static bool url_decode(const std::string& in, std::string& out);
        };
    } // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_HANDLER_HPP