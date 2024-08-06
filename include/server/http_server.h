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
#include <mutex>
#include <set>
#include <thread>
#include <vector>
#include <jsoncpp/json/value.h>
#include "content_type.h"
#include "http_server_config.h"
#include "http_method.h"
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

/**
 * @brief HTTP服务器快照结果.
 */
struct SnapshotResult {
    struct RequestInfo {
        size_t thread_id = 0;
        int64_t id = -1;
        HttpMethod method;
        ContentType content_type;
        std::shared_ptr<const Route> route;
        tp arrive_timepoint;
        std::string path;
        std::string client_ip;
        std::string client_real_ip;
    };

    /** 创建时间 */
    tp create_time;

    /** 当前会话数量 */
    uint32_t curr_num_sessions = 0;
    /** 当前工作线程数量 */
    uint32_t curr_num_worker_threads = 0;

    /** 总计创建的会话数量 */
    uint64_t total_num_sessions = 0;
    /** 总计处理的请求数量 */
    uint64_t total_num_requests = 0;

    /** 正在处理的请求 */
    std::vector<RequestInfo> handling_request;

    /** 转为JSON对象 */
    Json::Value ToJson() const;
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
    virtual ~HttpServer();

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
    int64_t current_request_id() { return current_request_id_.fetch_add(1); }
    void set_current_request_id(int64_t id) { current_request_id_.store(id); }
    std::shared_ptr<ILogger> logger() const { return logger_; }
    std::shared_ptr<Router> router() const { return router_; }
    bool should_stop() const { return should_stop_; }

    /**
     * @brief 启动服务器.
     */
    bool Start();

    /**
     * @brief 启动服务器(异步).
     */
    bool StartAsync();

    /**
     * @brief 停止服务器.
     */
    void Stop();

    /**
     * @brief 停止服务器(异步).
     */
    void StopAsync();

    /**
     * @brief 等待服务器停止.
     */
    void WaitForStop() const;

    /**
     * @brief 判断服务器是否已停止.
     */
    bool Stopped() const;

    /**
     * @brief 创建快照.
     */
    SnapshotResult CreateSnapshot();

public:
    /**
     * @brief 获取二进制程序所在目录，绝对路径，以'/'结尾.
     */
    static const std::string& GetBinDir();

    /**
     * @brief 获取二进制程序所在目录，绝对路径，以'/'结尾，UTF8编码.
     */
    static const std::string& GetBinDirUtf8();

private:
    /**
     * @brief 创建新的工作线程.
     */
    void NewWorkerThreads(uint32_t n);

    /**
     * @brief 工作线程.
     */
    void ThreadFunc_Worker();

    /**
     * @brief 管理者线程.
     */
    void ThreadFunc_Manager();

    void OnNewSession();
    void OnDestroySession();

    void OnStartRequest(Request* req);
    void OnFinishRequest(Request* req);

private:
    HttpServerConfig config_;
    std::atomic_int64_t current_request_id_{-1};

    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<boost::asio::io_context> ioc_;
    std::shared_ptr<Router> router_;
    std::vector<std::shared_ptr<Listener>> listeners_;

    std::mutex mutex_server_state_;
    bool is_running_;
    bool should_stop_;

    /** 当前会话数量 */
    std::atomic_uint32_t curr_num_sessions_;
    /** 当前工作线程数量 */
    std::atomic_uint32_t curr_num_worker_threads_;

    /** 总计创建的会话数量 */
    std::atomic_uint64_t total_num_sessions_;
    /** 总计处理的请求数量 */
    std::atomic_uint64_t total_num_requests_;

    std::mutex mutex_requests_;
    std::set<Request*> handling_requests_;

    std::function<bool(Request&, Response&)> cb_before_parse_body_;
    std::function<bool(Request&, Response&)> cb_before_handle_request_;
    std::function<bool(Request&, Response&)> cb_before_send_response_;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_HTTP_SERVER_H_
