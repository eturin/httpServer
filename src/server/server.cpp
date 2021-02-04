#include "server.hpp"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "spdlog/spdlog.h"

namespace http {
    namespace server3 {

        server::server(const std::string& address,
                       const std::string& port,
                       const std::string& doc_root,
                       std::size_t thread_pool_size,
                       Context &context) : thread_pool_size_(thread_pool_size),
                                                       signals_(io_service_),
                                                       acceptor_(io_service_),
                                                       new_connection_(),
                                                       request_handler_(doc_root,context),
                                                       context(context)
        {
            // Регистрируем обработчик сигналов прекращения работы сервера.
            // Можно регистрировать обработчики сигнала несколько раз при условие, что
            // регистрация через Asio.
            spdlog::info("Регистрирую обработчик сигнала прекращения работы сервера");
            signals_.add(SIGINT);
            signals_.add(SIGTERM);
#if defined(SIGQUIT)
            signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
            signals_.async_wait(boost::bind(&server::handle_stop, this));

            // открываем acceptor с опцией повторного использования адреса (т.е. SO_REUSEADDR).
            boost::asio::ip::tcp::resolver resolver(io_service_);
            boost::asio::ip::tcp::resolver::query query(address, port);
            boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
            spdlog::info("Настраиваю acceptor");
            acceptor_.open(endpoint.protocol());
            acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            acceptor_.bind(endpoint);
            acceptor_.listen();

            start_accept();
        }

        void server::run() {
            // создаем пул потоков
            spdlog::warn("Запуск сервера");
            std::vector<boost::shared_ptr<boost::thread> > threads;
            for (std::size_t i = 0; i < thread_pool_size_; ++i) {
                boost::shared_ptr<boost::thread> thread(new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_)));
                threads.push_back(thread);
                context.add_uuid_num(thread->get_id(), i);
            }

            // дожидаемся готовности потоков
            for (auto &e: threads)
                e->join();
        }

        void server::start_accept() {
            new_connection_.reset(new connection(io_service_, request_handler_));
            acceptor_.async_accept(new_connection_->socket(),
                                   boost::bind(&server::handle_accept,
                                                  this,
                                                      boost::asio::placeholders::error));
        }

        void server::handle_accept(const boost::system::error_code& e) {
            if (!e) new_connection_->start();

            start_accept();
        }

        void server::handle_stop() {
            spdlog::warn("Прекращение работы.");
            io_service_.stop();
        }
    } // namespace server3
} // namespace http