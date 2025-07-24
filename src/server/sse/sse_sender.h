#ifndef IC_SERVER_SSE_SSE_SENDER_H_
#define IC_SERVER_SSE_SSE_SENDER_H_
#include <atomic>
#include <functional>
#include <string>
#include <boost/beast/core.hpp>
#include "server/sse/sse_provider.h"

namespace ic {
namespace server {

class HttpServer;

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>

/**
 * @brief SSE事件发送.
 */
class SseSender : public std::enable_shared_from_this<SseSender> {
public:
    SseSender(beast::tcp_stream* stream_, HttpServer* svr, std::shared_ptr<SseProvider> sse_provider);
    ~SseSender();

    void Run(std::function<void(beast::error_code ec, uint64_t bytes_transfered)> onfinish);

private:
    std::shared_ptr<SseProvider> GetProvider() { return provider_.lock(); }
    std::weak_ptr<SseSender> GetWeakFromThis() { return shared_from_this(); }

    void TimerLoop();
    bool DoSendNextSseEvent();

    void OnFinish(beast::error_code ec);

private:
    HttpServer* svr_;
    beast::tcp_stream* stream_;
    std::weak_ptr<SseProvider> provider_;

    net::steady_timer timer_;
    std::atomic_bool is_sending_{false};
    std::string sending_event_;
    uint64_t bytes_transfered_ = 0;

    std::function<void(beast::error_code ec, uint64_t bytes_transfered)> onfinish_;
    std::atomic_bool is_finished_{false};
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_SSE_SSE_SENDER_H_
