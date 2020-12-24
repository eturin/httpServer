#ifndef HTTP_SERVER3_SERVER_HPP
#define HTTP_SERVER3_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "connection.hpp"
#include "request_handler.hpp"

namespace http {
    namespace server3 {
        class server : private boost::noncopyable {
        public:
            explicit server(const std::string& address,
                            const std::string& port,
                            const std::string& doc_root,
                            std::size_t thread_pool_size);
            void run();
        private:
            // асинхронное принятие соединений
            void start_accept();
            // обработка завершения асинхронной операции принятия.
            void handle_accept(const boost::system::error_code& e);
            // обработка запроса остановки сервера
            void handle_stop();
            // количество потоков сервера (передается в io_service::run())
            std::size_t thread_pool_size_;
            // Io_service, используемый для выполнения асинхронных операций.
            boost::asio::io_service io_service_;
            // signal_set используется для регистрации уведомлений о завершении процесса..
            boost::asio::signal_set signals_;
            // Acceptor используется для прослушивания входящих подключений.
            boost::asio::ip::tcp::acceptor acceptor_;
            // очередное сообщение, которое следует принять
            connection_ptr new_connection_;
            // Обработчик всех входящих запросов.
            request_handler request_handler_;
        };
    } // namespace server3
} // namespace http

#endif // HTTP_SERVER3_SERVER_HPP