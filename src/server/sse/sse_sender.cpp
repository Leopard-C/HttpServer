#include "sse_sender.h"
#include "server/http_server.h"
#include "server/sse/sse_provider.h"
#include <boost/asio/write.hpp>

namespace ic {
namespace server {

SseSender::SseSender(beast::tcp_stream* stream, HttpServer* svr, std::shared_ptr<SseProvider> sse_provider)
    : svr_(svr), stream_(stream), provider_(sse_provider), timer_(stream->get_executor())
{
}

SseSender::~SseSender() {
    OnFinish({});
}

void SseSender::Run(std::function<void(beast::error_code ec, uint64_t bytes_transfered)> onfinish) {
    onfinish_ = onfinish;

    auto provider = GetProvider();
    if (!provider) {
        return;
    }

    /* 订阅新事件通知 */
    auto weak_self = GetWeakFromThis();
    provider->Subscribe([weak_self] {
        if (auto shared_self = weak_self.lock()) {
            if (shared_self->is_sending_) {
                return;
            }
            /* 当前不在io线程，必须使用net::post转移到io线程进行发送 */
            net::post(shared_self->stream_->get_executor(), [weak_self] {
                if (auto shared_self = weak_self.lock()) {
                    shared_self->DoSendNextSseEvent();
                }
            });
        }
    });

    /* 持续发送响应内容 */
    if (DoSendNextSseEvent()) {
        TimerLoop();
    }
}

void SseSender::TimerLoop() {
    auto self = shared_from_this();
    timer_.expires_after(std::chrono::milliseconds(10));
    timer_.async_wait([self](beast::error_code ec) {
        if (ec) {
            return self->OnFinish(ec);
        }
        if (self->DoSendNextSseEvent()) {
            self->TimerLoop();
        }
    });
}

bool SseSender::DoSendNextSseEvent() {
    if (svr_->should_stop()) {
        OnFinish({});
        return false;
    }

    bool expected = false;
    if (!is_sending_.compare_exchange_strong(expected, true)) {
        return true;
    }

    const uint64_t max_bytes = 1024ULL * 256;  // 256KB
    auto provider = GetProvider();
    if (!provider->TryPopSome(max_bytes, &sending_event_)) {
        if (!provider->is_alive()) {
            OnFinish({});
            return false;
        }
        is_sending_ = false;
        return true;
    }

    auto self = shared_from_this();
    net::async_write(*stream_, net::buffer(sending_event_), [self](beast::error_code ec, std::size_t bytes_transfered) {
        self->bytes_transfered_ += bytes_transfered;
        if (ec) {
            return self->OnFinish(ec);
        }
        self->is_sending_ = false;
        if (auto provider = self->GetProvider()) {
            if (!provider->empty()) {
                net::post(self->stream_->get_executor(), [self]() { self->DoSendNextSseEvent(); });
            }
        }
    });
    return true;
}

void SseSender::OnFinish(beast::error_code ec) {
    bool expected = false;
    if (!is_finished_.compare_exchange_strong(expected, true)) {
        /* OnFinish 执行1次即可 */
        return;
    }

    if (auto provider = GetProvider()) {
        provider->Shutdown();
    }

    timer_.expires_after(std::chrono::seconds(-1));
    timer_.wait();

    if (onfinish_) {
        onfinish_(ec, bytes_transfered_);
        onfinish_ = nullptr;
    }
}

} // namespace server
} // namespace ic
