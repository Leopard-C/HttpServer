#ifndef IC_SERVER_SESSION_H_
#define IC_SERVER_SESSION_H_
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include "request.h"
#include "response.h"

namespace ic {
namespace server {

class HttpServer;

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket&& socket, HttpServer* svr);
    ~Session();

    void Run();
    void OnRead(beast::error_code ec, size_t bytes_transferred);
    void OnWrite(bool close, beast::error_code ec, size_t bytes_transferred);
    void DoRead();
    void DoClose();

    HttpServer* svr() { return svr_; }

private:
    void ProcessReadError(beast::error_code ec);
    bool PreHandleRequest();
    void HandleRequest();
    void SendResponse();
    void SendFileBodyResponse();
    void SendStringBodyResponse();

private:
    HttpServer* svr_{nullptr};
    bool close_{false};
    std::shared_ptr<http::request_parser<http::string_body>> parser_;
    std::shared_ptr<http::response_serializer<http::file_body>> file_serializer_;
    std::shared_ptr<Request> req_;
    std::shared_ptr<Response> res_;
    std::shared_ptr<http::response<http::string_body>> string_res_;
    std::shared_ptr<http::response<http::file_body>> file_res_;
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_SESSION_H_
