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
            // дескрипторы обработчика
            int fd_in[2]={0},
                    fd_out[2]={0},
                    fd_err[2]={0};
            // pid обработчика
            int pid=0;
            bool sync=false;
            ~connection();
        private:
            // обработчик завершения операции записи в socket
            void handle_write_socket(const boost::system::error_code& e);
            // обработчик завершения операции чтения из socket
            void handle_read_socket(const boost::system::error_code& e, std::size_t bytes_transferred);

            // обработчик завершения операции записи в stream
            void handle_write_stream(const boost::system::error_code& e);
            // обработчик завершения операции чтения из tream
            void handle_read_stream(const boost::system::error_code& e, std::size_t bytes_transferred);

            // для предотвращения вызова двух обработчиков соединения одновременно
            boost::asio::io_service::strand strand_;
            // главный такой сервис
            boost::asio::io_service& io_service;
            // socket
            boost::asio::ip::tcp::socket socket_;
            // stream
            boost::asio::posix::stream_descriptor *stream_in = nullptr,
                                                  *stream_out = nullptr,
                                                  *stream_err = nullptr;
            // обработчик входящего запроса.
            request_handler& request_handler_;
            // буфер входящих данных
            boost::array<char, 8192> buffer_;
            // входящий запрос
            request request_;
            // парсер входящего запроса
            request_parser request_parser_;
            // ответ для отправки клиенту
            reply reply_;
        };

        typedef boost::shared_ptr<connection> connection_ptr;

    } // namespace server3
} // namespace http

#endif // HTTP_SERVER3_CONNECTION_HPP