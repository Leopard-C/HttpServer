#ifndef IC_SERVER_SSE_SSE_SESSION_HANDLER_H_
#define IC_SERVER_SSE_SSE_SESSION_HANDLER_H_
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
 * @brief SSE事件.
 */
class SseSessionHandler : public std::enable_shared_from_this<SseSessionHandler> {
public:
    SseSessionHandler(beast::tcp_stream* stream_, HttpServer* svr, std::shared_ptr<SseProvider> sse_provider);
    ~SseSessionHandler() = default;

    void Handle(std::function<void(beast::error_code ec, uint64_t bytes_transfered)> onfinish);

public:
    void TimerLoop();
    bool DoSendNextSseEvent();

    void OnFinish(beast::error_code ec);

private:
    HttpServer* svr_;
    beast::tcp_stream* stream_;
    std::shared_ptr<SseProvider> provider_;

    std::function<void(beast::error_code ec, uint64_t bytes_transfered)> onfinish_;
    std::atomic_bool is_finished_{false};

    net::steady_timer timer_;
    std::atomic_bool is_sending_{false};
    std::string sending_event_;
    uint64_t bytes_transfered_ = 0;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_SSE_SSE_SESSION_HANDLER_H_
