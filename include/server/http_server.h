/**
 * @file http_server.h
 * @brief HttpServer based on boost.beast.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_SERVER_HTTP_SERVER_H_
#define IC_SERVER_HTTP_SERVER_H_
#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <jsoncpp/json/value.h>
#include "config.h"
#include "logger.h"

namespace boost {
namespace asio {
    class io_context;
}
}

namespace ic {
namespace server {

class Listener;
class Route;
class Router;
class Session;
class Request;
class Response;
class HttpServer;

using tp = std::chrono::system_clock::time_point;

class ThreadInfo {
public:
    ThreadInfo(HttpServer* svr);
    Json::Value ToJson(tp now = std::chrono::system_clock::now()) const;
    /** 当前线程是否被激活(正在处理请求) */
    bool active;
    /** 当前线程ID */
    size_t thread_id;
    /** 当前线程正在处理的请求的ID */
    int64_t request_id;
    /** 当前请求命中的路由 */
    const Route* route;
    /** 开始处理请求时间 */
    tp start;
    /** 结束处理请求时间 */
    tp finish;
    /** 当前线程总共处理的请求数量 */
    uint64_t count;
private:
    const HttpServer* svr_{nullptr};
};

/**
 * @brief HTTP服务器.
 */
class HttpServer {
public:
    friend class Listener;
    friend class Session;

    /**
     * @brief 构造函数.
     * @param config 服务器配置参数
     * @param logger 日志记录器，为空则使用默认的日志记录器
     */
    HttpServer(const HttpServerConfig& config, std::shared_ptr<ILogger> logger = nullptr);
    ~HttpServer();

public:
    /**
     * @brief 设置请求拦截器：解析body内容之前调用.
     */
    void set_cb_before_parse_body(std::function<bool(Request&, Response&)> cb) { cb_before_parse_body_ = cb; }

    /**
     * @brief 设置请求拦截器：解析body内容之后，处理请求之前调用.
     */
    void set_cb_before_handle_request(std::function<bool(Request&, Response&)> cb) { cb_before_handle_request_ = cb; }

    /**
     * @brief 设置响应拦截器：返回响应内容之前调用.
     */
    void set_cb_before_send_response(std::function<bool(Request&, Response&)> cb) { cb_before_send_response_ = cb; }

    const HttpServerConfig& config() const { return config_; }
    std::shared_ptr<ILogger> logger() const { return logger_; }
    std::shared_ptr<Router> router() const { return router_; }
    int64_t current_request_id() { return current_request_id_.fetch_add(1); }
    void set_current_request_id(int64_t id) { current_request_id_ = id; }
    std::map<size_t, ThreadInfo> thread_infos();

    Json::Value DumpThreadInfos();

    /**
     * @brief 开始监听.
     * 
     * @retval true 监听成功
     * @retval false 监听失败（如无效的端口、端口被占用等原因）
     */
    bool Listen();

    /**
     * @brief 启动服务器（阻塞当前线程）.
     */
    void Start();

    /**
     * @brief 停止服务器（非阻塞）.
     */
    void Stop();

    /**
     * @brief 判断服务器是否已停止.
     */
    bool Stopped() const;

    /**
     * @brief 获取二进制程序所在目录，绝对路径，以'/'结尾.
     */
    static const std::string& GetBinDir();

private:
    void RunIoContext();
    ThreadInfo* GetThreadInfo(size_t tid);

    void StartRequest(Request& req);
    void FinishRequest(Request& req);

private:
    HttpServerConfig config_;
    std::shared_ptr<ILogger> logger_;
    std::atomic_int64_t current_request_id_{-1};
    std::shared_ptr<boost::asio::io_context> ioc_;
    std::shared_ptr<Listener> listener_;
    std::shared_ptr<Router> router_;
    std::vector<std::thread> worker_threads_;

    std::mutex mutex_for_thread_infos_;
    std::map<size_t, ThreadInfo*> thread_infos_;

    std::function<bool(Request&, Response&)> cb_before_parse_body_;
    std::function<bool(Request&, Response&)> cb_before_handle_request_;
    std::function<bool(Request&, Response&)> cb_before_send_response_;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_HTTP_SERVER_H_
