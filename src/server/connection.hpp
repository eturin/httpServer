#ifndef HTTP_SERVER3_CONNECTION_HPP
#define HTTP_SERVER3_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "reply.hpp"
#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"

namespace http {
    namespace server3 {
        //отдельное соединение
        class connection : public boost::enable_shared_from_this<connection>, private boost::noncopyable {
        public:
            explicit connection(boost::asio::io_service& io_service,
                                request_handler& handler);
            // socket соединения
            boost::asio::ip::tcp::socket& socket();
            // запуск первой асинхронной операции для соединения
            void start();
        private:
            // обработчик завершения операции чтения
            void handle_read(const boost::system::error_code& e, std::size_t bytes_transferred);
            // обработчик завершения операции записи.
            void handle_write(const boost::system::error_code& e);

            // для предотвращения вызова двух обработчиков соединения одновременно
            boost::asio::io_service::strand strand_;
            // socket
            boost::asio::ip::tcp::socket socket_;
            // обработчик, используемый для обработки входящего запроса.
            request_handler& request_handler_;
            // буфер входящих данных
            boost::array<char, 8192> buffer_;
            // входящий запрос
            request request_;
            // обработчик входящего запроса
            request_parser request_parser_;
            /// ответ для отправки клиенту
            reply reply_;
        };

        typedef boost::shared_ptr<connection> connection_ptr;

    } // namespace server3
} // namespace http

#endif // HTTP_SERVER3_CONNECTION_HPP