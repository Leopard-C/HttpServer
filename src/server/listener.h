#ifndef IC_SERVER_LISTENER_H_
#define IC_SERVER_LISTENER_H_
#include <vector>
#include <boost/beast/core.hpp>

namespace ic {
namespace server {

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = net::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

class HttpServer;

/**
 * @brief 监听器，接受连接请求.
 */
class Listener : public std::enable_shared_from_this<Listener> {
public:
    Listener(HttpServer* svr);
    ~Listener() = default;

    bool Run(const std::string& ip, unsigned short port, bool reuse_address);

    bool is_running() const { return is_running_; }
    const tcp::acceptor& acceptor() const { return acceptor_; }

private:
    void DoAccept();
    void OnAccept(beast::error_code ec, tcp::socket socket);

private:
    HttpServer* svr_;
    bool is_running_;
    tcp::acceptor acceptor_;
};

} // namesapce server
} // namespace ic

#endif // IC_SERVER_LISTENER_H_
