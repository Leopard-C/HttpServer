#include "sse_session_handler.h"
#include "server/http_server.h"
#include "server/sse/sse_provider.h"
#include <boost/asio/write.hpp>

namespace ic {
namespace server {

SseSessionHandler::SseSessionHandler(beast::tcp_stream* stream, HttpServer* svr, std::shared_ptr<SseProvider> sse_provider)
    : svr_(svr), stream_(stream), provider_(sse_provider), timer_(stream->get_executor())
{
}

void SseSessionHandler::Handle(std::function<void(beast::error_code ec, uint64_t bytes_transfered)> onfinish) {
    onfinish_ = onfinish;

    /* 订阅新事件通知 */
    provider_->Subscribe([self = weak_from_this()] {
        if (auto shared_self = self.lock()) {
            shared_self->DoSendNextSseEvent();
        }
    });

    /* 持续发送响应内容 */
    if (DoSendNextSseEvent()) {
        TimerLoop();
    }
}

void SseSessionHandler::TimerLoop() {
    timer_.expires_after(std::chrono::milliseconds(10));
    timer_.async_wait([self = shared_from_this()](beast::error_code ec) {
        if (ec) {
            return self->OnFinish(ec);
        }
        if (self->DoSendNextSseEvent()) {
            self->TimerLoop();
        }
    });
}

bool SseSessionHandler::DoSendNextSseEvent() {
    if (svr_->should_stop()) {
        beast::error_code ec;
        OnFinish(ec);
        return false;
    }

    bool expected = false;
    if (!is_sending_.compare_exchange_strong(expected, true)) {
        return true;
    }
    const uint64_t max_bytes = 1024ULL * 256;  // 256KB
    if (!provider_->TryPopSome(max_bytes, &sending_event_)) {
        if (!provider_->is_alive()) {
            beast::error_code ec;
            OnFinish(ec);
            return false;
        }
        is_sending_ = false;
        return true;
    }

    net::async_write(*stream_, net::buffer(sending_event_), [self = shared_from_this()](beast::error_code ec, std::size_t bytes_transfered) {
        self->bytes_transfered_ += bytes_transfered;
        if (ec) {
            return self->OnFinish(ec);
        }
        self->is_sending_ = false;
        if (!self->provider_->empty()) {
            net::post(self->stream_->get_executor(), [self]() { self->DoSendNextSseEvent(); });
        }
    });
    return true;
}

void SseSessionHandler::OnFinish(beast::error_code ec) {
    bool expected = false;
    if (!is_finished_.compare_exchange_strong(expected, true)) {
        return;
    }
    provider_->Shutdown(true);
    timer_.expires_after(std::chrono::seconds(-1));
    timer_.wait();
    if (onfinish_) {
        onfinish_(ec, bytes_transfered_);
    }
}

} // namespace server
} // namespace ic
