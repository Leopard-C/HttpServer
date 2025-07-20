#include "server/sse/sse_provider.h"

namespace ic {
namespace server {

SseProvider::SseProvider() : heartbeat_event_(": \n\n"), heartbeat_interval_(-1) {}

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
 * @brief 添加事件到队列中.
 * @param event 事件
 */
void SseProvider::Push(const SseEvent& event) {
    std::lock_guard<std::mutex> lck(mutex_);
    queue_.emplace(event.Serialize());
}

/**
 * @brief 清空队列.
 */
void SseProvider::Clear() {
    std::lock_guard<std::mutex> lck(mutex_);
    std::queue<std::string>().swap(queue_);
}

/**
 * @brief 断开连接(停止提供内容).
 */
void SseProvider::Shutdown() {
    std::lock_guard<std::mutex> lck(mutex_);
    is_alive_ = false;
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
 * @brief 检查消息队列是否为空.
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
 * @brief 等待直到获取到事件.
 * @param[in] max_wait_time_ms 最长等待时间(毫秒)
 * @param[out] event 获取到的队列头部的事件
 * @return 是否成功获取
 */
bool SseProvider::WaitAndPop(int64_t max_wait_time_ms, std::string* event) {
    std::unique_lock<std::mutex> lck(mutex_);
    cv_.wait_for(lck, std::chrono::milliseconds(max_wait_time_ms), [&] { return !queue_.empty() || !is_alive_; });
    if (!is_alive_) {
        return false;
    }
    auto now = std::chrono::steady_clock::now();
    if (!queue_.empty()) {
        event->swap(queue_.front());
        queue_.pop();
        last_timepoint_pop_event_ = now;
        return true;
    }
    if (heartbeat_interval_ > -1 && (now - last_timepoint_pop_event_).count() >= heartbeat_interval_ * 1000000) {
        *event = heartbeat_event_;
        last_timepoint_pop_event_ = now;
        return true;
    }
    return false;
}

} // namespace server
} // namespace ic

