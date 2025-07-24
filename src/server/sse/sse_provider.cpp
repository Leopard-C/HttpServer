#include "server/sse/sse_provider.h"

namespace ic {
namespace server {

SseProvider::SseProvider() : heartbeat_event_(": \n\n"), heartbeat_interval_(-1) {}

std::shared_ptr<SseProvider> SseProvider::Create() {
    return std::make_shared<SseProvider>();
}

/**
 * @brief 添加事件到队列中.
 * @param event 事件
 * @return 是否添加成功(如果事件积压达到上限，或者已调用过Shutdown，将返回false)
 */
bool SseProvider::Push(const SseEvent& event) {
    std::function<void()> callback;
    {
        std::lock_guard<std::mutex> lck(mutex_);
        if (!is_alive_ || queue_.size() >= max_queue_length_) {
            return false;
        }
        queue_.emplace(event.Serialize());
        callback = subscribed_callback_;
    }
    if (callback) {
        callback();
    }
    return true;
}

/**
 * @brief 清空队列.
 */
void SseProvider::Clear() {
    std::lock_guard<std::mutex> lck(mutex_);
    std::queue<std::string>().swap(queue_);
}

/**
 * @brief 断开连接.
 * @param immediate 是否立即断开(清空事件队列)
 */
void SseProvider::Shutdown(bool immediate/* = false*/) {
    std::function<void()> callback;
    {
        std::lock_guard<std::mutex> lck(mutex_);
        is_alive_ = false;
        callback.swap(subscribed_callback_);
        if (immediate) {
            std::queue<std::string>().swap(queue_);
        }
    }
    if (callback) {
        callback();
    }
}

/**
 * @brief 订阅事件通知.
 */
void SseProvider::Subscribe(std::function<void()> callback) {
    std::lock_guard<std::mutex> lck(mutex_);
    subscribed_callback_ = callback;
}

/**
 * @brief 取消订阅事件通知.
 */
void SseProvider::Unsubscribe() {
    std::lock_guard<std::mutex> lck(mutex_);
    subscribed_callback_ = nullptr;
}

/**
 * @brief 设置事件队列最大长度.
 */
void SseProvider::set_max_queue_length(size_t max_length) {
    std::lock_guard<std::mutex> lck(mutex_);
    max_queue_length_ = max_length;
}

/**
 * @brief 设置心跳包事件(可选).
 * @param heartbeat_event 心跳包内容
 * @param interval_ms 心跳包发送间隔(毫秒)
 * @note 默认心跳包内容 ": \n\n" 共4个字节
 */
void SseProvider::set_heartbeat_event(const SseEvent& heartbeat_event, int64_t interval_ms) {
    std::lock_guard<std::mutex> lck(mutex_);
    heartbeat_event_ = heartbeat_event.Serialize();
    heartbeat_interval_ = interval_ms;
}

/**
 * @brief 设置心跳包发送间隔(可选).
 * @param interval_ms 间隔(毫秒), -1表示不发送心跳包
 * @note 默认间隔-1, 不发送心跳包
 */
void SseProvider::set_heartbeat_interval(int64_t interval_ms) {
    std::lock_guard<std::mutex> lck(mutex_);
    heartbeat_interval_ = interval_ms;
}

/**
 * @brief 事件队列长度最大值.
 */
size_t SseProvider::max_queue_length() const {
    std::lock_guard<std::mutex> lck(mutex_);
    return max_queue_length_;
}

/**
 * @brief 获取心跳包事件.
 */
std::string SseProvider::heartbeat_event() const {
    std::lock_guard<std::mutex> lck(mutex_);
    return heartbeat_event_;
}

/**
 * @brief 获取心跳包发送间隔(毫秒).
 */
int64_t SseProvider::heartbeat_interval() const {
    std::lock_guard<std::mutex> lck(mutex_);
    return heartbeat_interval_;
}

/**
 * @brief 判断连接是有效.
 */
bool SseProvider::is_alive() const {
    std::lock_guard<std::mutex> lck(mutex_);
    return is_alive_;
}

/**
 * @brief 检查事件队列是否为空.
 */
bool SseProvider::empty() const {
    std::lock_guard<std::mutex> lck(mutex_);
    return queue_.empty();
}

/**
 * @brief 获取消息队列长度.
 */
size_t SseProvider::size() const {
    std::lock_guard<std::mutex> lck(mutex_);
    return queue_.size();
}

/**
 * @brief 尝试获取队列头部的事件.
 * @param[out] event 获取到的事件
 * @return 是否获取到事件
 */
bool SseProvider::TryPop(std::string* event) {
    std::lock_guard<std::mutex> lck(mutex_);
    if (!queue_.empty()) {
        event->swap(queue_.front());
        queue_.pop();
        last_timepoint_pop_event_ = std::chrono::steady_clock::now();
        return true;
    }
    else if (is_alive_) {
        return TryGetHeartbeatEvent(event);
    }
    else {
        return false;
    }
}

/**
 * @brief 尝试获取队列头部的多个事件并进行合并.
 * @param[in] max_bytes 合并后的事件大小最大字节数
 * @param[out] events 获取到的多个事件合并结果
 * @return 是否获取到事件
 */
bool SseProvider::TryPopSome(uint64_t max_bytes, std::string* events) {
    std::lock_guard<std::mutex> lck(mutex_);
    if (!queue_.empty()) {
        events->swap(queue_.front());
        queue_.pop();
        while (!queue_.empty() && events->size() + queue_.front().size() <= max_bytes) {
            *events += queue_.front();
            queue_.pop();
        }
        last_timepoint_pop_event_ = std::chrono::steady_clock::now();
        return true;
    }
    else if (is_alive_) {
        return TryGetHeartbeatEvent(events);
    }
    else {
        return false;
    }
}

/**
 * @brief 尝试获取心跳包事件.
 * @param[out] event 获取到的事件
 * @note 加锁后调用
 */
bool SseProvider::TryGetHeartbeatEvent(std::string* event) {
    if (heartbeat_interval_ < 0) {
        return false;
    }
    auto now = std::chrono::steady_clock::now();
    auto diff_ns = (now - last_timepoint_pop_event_).count();
    if (diff_ns < heartbeat_interval_ * 1000000) {
        return false;
    }
    *event = heartbeat_event_;
    last_timepoint_pop_event_ = now;
    return true;
}

} // namespace server
} // namespace ic

