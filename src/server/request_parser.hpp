#ifndef HTTP_SERVER3_REQUEST_PARSER_HPP
#define HTTP_SERVER3_REQUEST_PARSER_HPP

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>

namespace http {
    namespace server3 {
        struct request;

        // Разбор входящего зароса
        class request_parser {
        public:
            request_parser();

            // сброс состояния
            void reset();

            /* разбирает входящий запрос. Возвращаемое значение (tribool) равно true,
             * если разбор выполнен, и false, если запрос не валидный или требуется больше данных.
             * Возвращаемое значение InputIterator указывает, какая часть ввода была использована. */
            template <typename InputIterator>
            boost::tuple<boost::tribool, InputIterator> parse(request& req,
                                                              InputIterator begin, InputIterator end)
            {
                while (begin != end) {
                    boost::tribool result = consume(req, *begin++);
                    if (result || !result)
                        return boost::make_tuple(result, begin);
                }
                boost::tribool result = boost::indeterminate;
                return boost::make_tuple(result, begin);
            }
        private:

            // обработка следующего символа запроса (input)
            boost::tribool consume(request& req, char input);

            // проверка на символ HTTP
            static bool is_char(int c);

            // проверка на управляющий символ
            static bool is_ctl(int c);

            // проверка наспециальный символ
            static bool is_tspecial(int c);

            // проверка на цифру
            static bool is_digit(int c);

            // состояние в настоящий момент
            enum state {
                method_start,
                method,
                uri,
                http_version_h,
                http_version_t_1,
                http_version_t_2,
                http_version_p,
                http_version_slash,
                http_version_major_start,
                http_version_major,
                http_version_minor_start,
                http_version_minor,
                expecting_newline_1,
                header_line_start,
                header_lws,
                header_name,
                space_before_header_value,
                header_value,
                expecting_newline_2,
                expecting_newline_3,
                body
            } state_;
        };

    } // namespace server3
} // namespace http

#endif // HTTP_SERVER3_REQUEST_PARSER_HPP