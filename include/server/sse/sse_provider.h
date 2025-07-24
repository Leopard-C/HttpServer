#ifndef IC_SERVER_SSE_SSE_PROVIDER_H_
#define IC_SERVER_SSE_SSE_PROVIDER_H_
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include "sse_event.h"

namespace ic {
namespace server {

/**
 * @brief SSE(Server-Sent Event)内容提供者.
 * @details 主要功能是一个线程安全的事件队列.
 */
class SseProvider {
public:
    SseProvider();
    SseProvider(const SseProvider& rhs) = delete;
    SseProvider(SseProvider&& rhs) = delete;
    SseProvider& operator=(const SseProvider& rhs) = delete;
    ~SseProvider() = default;

    /**
     * @brief 创建智能指针对象.
     */
    static std::shared_ptr<SseProvider> Create();

    /**
     * @brief 添加事件到队列中.
     * @param event 事件
     * @return 是否添加成功(如果事件积压达到上限，或者已调用过Shutdown，将返回false)
     * @{
     */
    bool Push(const SseEvent& event);
    bool Push(const std::string& event);
    bool Push(std::string&& event);
    /** @} */

    /**
     * @brief 清空事件队列.
     */
    void Clear();

    /**
     * @brief 断开连接.
     * @param immediate 是否立即断开(清空事件队列)
     */
    void Shutdown(bool immediate = false);

    /**
     * @brief 订阅事件通知.
     * @param callback 回调函数
     */
    void Subscribe(std::function<void()> callback);

    /**
     * @brief 取消订阅事件通知.
     */
    void Unsubscribe();

    /**
     * @brief 尝试获取队列头部的事件.
     * @param[out] event 获取到的事件
     * @return 是否获取到事件
     */
    bool TryPop(std::string* event);

    /**
     * @brief 尝试获取队列头部的多个事件并进行合并.
     * @param[in] max_bytes 合并后的事件大小最大字节数
     * @param[out] events 获取到的多个事件合并结果
     * @return 是否获取到事件
     */
    bool TryPopSome(uint64_t max_bytes, std::string* events);

    /**
     * @brief 设置事件队列最大长度.
     */
    void set_max_queue_length(size_t max_length);

    /**
     * @brief 设置心跳包事件(可选).
     * @param heartbeat_event 心跳包内容
     * @param interval_ms 心跳包发送间隔(毫秒)
     * @note 默认心跳包内容 ": \n\n" 共4个字节
     */
    void set_heartbeat_event(const SseEvent& heartbeat_event, int64_t interval_ms);

    /**
     * @brief 设置心跳包发送间隔(可选).
     * @param interval_ms 间隔(毫秒), -1表示不发送心跳包
     * @note 默认间隔-1, 不发送心跳包
     */
    void set_heartbeat_interval(int64_t interval_ms);

public:
    /**
     * @brief 事件队列最大长度.
     */
    size_t max_queue_length() const;

    /**
     * @brief 获取心跳包事件.
     */
    std::string heartbeat_event() const;

    /**
     * @brief 获取心跳包发送间隔(毫秒).
     */
    int64_t heartbeat_interval() const;

    /**
     * @brief 判断连接是有效.
     */
    bool is_alive() const;

    /**
     * @brief 检查事件队列是否为空.
     */
    bool empty() const;

    /**
     * @brief 获取消息队列长度.
     */
    size_t size() const;

private:
    /**
     * @brief 尝试获取心跳包事件.
     * @param[out] event 获取到的事件
     * @note 加锁后调用
     */
    bool TryGetHeartbeatEvent(std::string* event);

private:
    mutable std::mutex mutex_;
    bool is_alive_{true};

    std::queue<std::string> queue_;
    size_t max_queue_length_{256};

    std::function<void()> subscribed_callback_;

    std::string heartbeat_event_;
    int64_t heartbeat_interval_{-1};
    std::chrono::steady_clock::time_point last_timepoint_pop_event_;
}; // class SseProvider

} // namespace server
} // namespace ic

#endif // IC_SERVER_SSE_SSE_PROVIDER_H_
