#include "connection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include <wait.h>
#include "request_handler.hpp"
#include "spdlog/spdlog.h"

namespace http {
    namespace server3 {
        connection::connection(boost::asio::io_service& io_service,
                               request_handler& handler) : io_service(io_service),
                                                           strand_(io_service),
                                                           socket_(io_service),
                                                           request_handler_(handler),
                                                           start_time(std::chrono::system_clock::now())
        {
        }

        connection::~connection() {
            delete stream_out, stream_err, stream_in;
            close(fd_in[1]);
            close(fd_out[0]);
            close(fd_err[0]);

            if (pid) //убиваем обработчик
                kill(pid,9);
        }
        boost::asio::ip::tcp::socket& connection::socket() {
            return socket_;
        }

        void connection::start() {
            socket_.async_read_some(boost::asio::buffer(buffer_),
                                    strand_.wrap(boost::bind(&connection::handle_read_socket,
                                                                       shared_from_this(),
                                                                          boost::asio::placeholders::error,
                                                                          boost::asio::placeholders::bytes_transferred)));
        }

        void connection::handle_read_socket(const boost::system::error_code& e,
                                            std::size_t bytes_transferred) {
            if (!e) {
                boost::tribool result;
                boost::tie(result, boost::tuples::ignore) = request_parser_.parse(request_,
                                                                                     buffer_.data(),
                                                                                      buffer_.data() + bytes_transferred);

                if (result) {
                    if (request_handler_.handle_request(request_, reply_, this)) {
                        boost::asio::async_write(socket_,
                                                 reply_.to_buffers(),
                                                 strand_.wrap(boost::bind(&connection::handle_write_socket,
                                                                                     shared_from_this(),
                                                                                         boost::asio::placeholders::error)));
                    } else {
                        std::vector<boost::asio::const_buffer> vbody;
                        vbody.push_back(boost::asio::buffer(request_.body));
                        stream_in = new boost::asio::posix::stream_descriptor(io_service, fd_in[1]);
                        stream_in->async_write_some(vbody,
                                                   strand_.wrap(boost::bind(&connection::handle_write_stream,
                                                                                         shared_from_this(),
                                                                                          boost::asio::placeholders::error)));
                    }
                } else if (!result) {
                    reply_ = reply::stock_reply(reply::bad_request);
                    boost::asio::async_write(socket_,
                                         reply_.to_buffers(),
                                         strand_.wrap(boost::bind(&connection::handle_write_socket,
                                                                            shared_from_this(),
                                                                               boost::asio::placeholders::error)));
                } else {
                    socket_.async_read_some(boost::asio::buffer(buffer_),
                                            strand_.wrap(boost::bind(&connection::handle_read_socket,
                                                                               shared_from_this(),
                                                                                  boost::asio::placeholders::error,
                                                                                  boost::asio::placeholders::bytes_transferred)));
                }
            }

            /* Если возникает ошибка, новые асинхронные операции не запускаются.
             * Это означает, что все ссылки shared_ptr на объект подключения исчезнут,
             * и объект будет автоматически уничтожен после возврата из этого обработчика.
             * Деструктор класса соединения закрывает сокет.*/
        }

        void connection::handle_write_socket(const boost::system::error_code& e) {
            if (!e) { // инициализация закрытия соединения
                boost::system::error_code ignored_ec;
                socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
            }

            /* Никаких новых асинхронных операций не запускается.
             * Это означает, что все ссылки shared_ptr на объект соединения исчезнут,
             * и объект будет автоматически уничтожен после возврата из этого обработчика.
             * Деструктор класса соединения закрывает сокет.*/
        }

        void connection::handle_read_stream(const boost::system::error_code &e, std::size_t bytes_transferred) {
            if (fd_out[0]) {
                reply_.content.append(buffer_.data(), buffer_.data() + bytes_transferred);

                if (!e) {
                    // продолжаем читать stdout пока не закроется
                    stream_out->async_read_some(boost::asio::buffer(buffer_),
                                                strand_.wrap(boost::bind(&connection::handle_read_stream,
                                                                         shared_from_this(),
                                                                         boost::asio::placeholders::error,
                                                                         boost::asio::placeholders::bytes_transferred)));
                } else {
                    stream_out->close();
                    delete  stream_out;
                    fd_out[0] = 0;
                    stream_out = nullptr;

                    // читаем stderr
                    stream_err = new boost::asio::posix::stream_descriptor(io_service, fd_err[0]);
                    stream_err->async_read_some(boost::asio::buffer(buffer_),
                                                strand_.wrap(boost::bind(&connection::handle_read_stream,
                                                                          shared_from_this(),
                                                                           boost::asio::placeholders::error,
                                                                           boost::asio::placeholders::bytes_transferred)));
                }
                return;
            }

            if (fd_err[0]) {
                if (!e) {
                    reply_.content.append(buffer_.data(), buffer_.data() + bytes_transferred);
                    // продолжаем читать stdout пока не закроется
                    reply_ = reply::stock_reply(reply::bad_request);
                    reply_.content.append(buffer_.data(), buffer_.data() + bytes_transferred);
                    stream_err->async_read_some(boost::asio::buffer(buffer_),
                                                strand_.wrap(boost::bind(&connection::handle_read_stream,
                                                                          shared_from_this(),
                                                                           boost::asio::placeholders::error,
                                                                           boost::asio::placeholders::bytes_transferred)));
                    return;
                } else {
                    stream_err->close();
                    delete  stream_err;
                    fd_err[0] = 0;
                    stream_err = nullptr;
                }
            }

            // останавливаем обработчик
            int  status;
            if (waitpid(pid,&status,WNOHANG)) pid = 0;

            // отправляем сформированный ответ
            reply_.headers[0].name = "Content-Length";
            reply_.headers[0].value = std::to_string(reply_.content.size());
            reply_.headers[1].name = "Content-Type";
            reply_.headers[1].value = "text";
            boost::asio::async_write(socket_,
                                     reply_.to_buffers(),
                                     strand_.wrap(boost::bind(&connection::handle_write_socket,
                                                                           shared_from_this(),
                                                                            boost::asio::placeholders::error)));

        }
        void connection::handle_write_stream(const boost::system::error_code &e) {
            stream_in->close();
            delete stream_in;
            stream_in = nullptr;
            fd_in[1]=0;

            reply_ = reply::stock_reply(reply::ok);
            if (sync) {
                // читаем stdout
                stream_out = new boost::asio::posix::stream_descriptor(io_service, fd_out[0]);
                stream_out->async_read_some(boost::asio::buffer(buffer_),
                                            strand_.wrap(boost::bind(&connection::handle_read_stream,
                                                                                  shared_from_this(),
                                                                                   boost::asio::placeholders::error,
                                                                                   boost::asio::placeholders::bytes_transferred)));
            } else {
                pid = 0;
                reply_.status = reply::not_implemented;

                reply_.content.append("Запрос принят на асинхронную обработку");

                reply_.headers.resize(2);
                reply_.headers[0].name = "Content-Length";
                reply_.headers[0].value = std::to_string(reply_.content.size());
                reply_.headers[1].name = "Content-Type";
                reply_.headers[1].value = "text";

                boost::asio::async_write(socket_,
                                         reply_.to_buffers(),
                                         strand_.wrap(boost::bind(&connection::handle_write_socket,
                                                                               shared_from_this(),
                                                                                boost::asio::placeholders::error)));

            }
        }
    } // namespace server3
} // namespace http