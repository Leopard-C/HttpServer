#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include "server/http_server.h"
#include "server/logger.h"
#include "server/request.h"
#include "server/router.h"
#include "server/util/format_time.h"
#include "server/util/path.h"
#include "server/util/thread.h"
#include "listener.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace ic {
namespace server {

constexpr uint32_t NUM_THREADS_LIMIT = 1000;

template<typename T>
static T s_clamp(T val, T lower, T upper) {
    return std::max(lower, std::min(val, upper));
}

/*******************************************************************
**
**                        SnapshotResult
** 
*******************************************************************/

/**
 * @brief 转为JSON对象.
 */
Json::Value SnapshotResult::ToJson() const {
    Json::Value root;
    root["create_time"] = util::format_time_us(create_time);
    root["curr_num_sessions"] = curr_num_sessions;
    root["curr_num_worker_threads"] = curr_num_worker_threads;
    root["total_num_sessions"] = total_num_sessions;
    root["total_num_requests"] = total_num_requests;
    auto& v_handling_requests = root["handling_requests"];
    v_handling_requests.resize(0);
    for (const auto& request : handling_request) {
        Json::Value v_handling_request;
        v_handling_request["id"] = request.id;
        v_handling_request["method"] = to_string(request.method);
        v_handling_request["content_type"] = request.content_type.ToString();
        v_handling_request["arrive_timepoint"] = util::format_time_us(request.arrive_timepoint);
        v_handling_request["path"] = request.path;
        v_handling_request["route"] = request.route->ToJson();
        v_handling_request["client_ip"] = request.client_ip;
        v_handling_request["client_real_ip"] = request.client_real_ip;
        v_handling_requests.append(v_handling_request);
    }
    return root;
}

/*******************************************************************
**
**                          HttpServer
** 
*******************************************************************/

HttpServer::HttpServer(const HttpServerConfig& config, std::shared_ptr<ILogger> logger/* = nullptr*/) :
    config_(config), logger_(logger), is_running_(false), should_stop_(false),
    curr_num_sessions_{ 0 }, curr_num_worker_threads_{ 0 }, total_num_requests_{0}
{
    if (!logger_) {
        logger_ = std::make_shared<ConsoleLogger>(LogLevel::kInfo, LogLevel::kWarn);
    }
    ioc_ = std::make_shared<net::io_context>(config.max_num_threads());
    router_ = std::make_shared<Router>(this);
}

HttpServer::~HttpServer() {
    Stop();
}

/**
 * @brief 启动服务器.
 */
bool HttpServer::Start() {
    if (!StartAsync()) {
        return false;
    }
    while (is_running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return true;
}

/**
 * @brief 启动服务器(异步).
 */
bool HttpServer::StartAsync() {
    if (config_.endpoints().empty()) {
        logger_->Error(LOG_CTX, "Invalid http server configuration. The endpoints is empty");
        return false;
    }
    if (config_.min_num_threads() < 1 || config_.min_num_threads() > config_.max_num_threads()) {
        logger_->Error(LOG_CTX, "Invalid http server configuration. The min_num_threads(%u) or max_num_threads(%u) is invalid", config_.min_num_threads(), config_.max_num_threads());
        return false;
    }
    if (config_.max_num_threads() > NUM_THREADS_LIMIT) {
        logger_->Error(LOG_CTX, "Invalid http server configuration. The max_num_threads(%u) is over %u", config_.max_num_threads(), NUM_THREADS_LIMIT);
        return false;
    }

    {
        std::lock_guard<std::mutex> lck(mutex_server_state_);
        if (is_running_) {
            return true;
        }

        /* 启动监听器 */
        for (size_t i = 0; i < config_.endpoints().size(); ++i) {
            auto& endpoint = config_.endpoints()[i];
            auto listener = std::make_shared<Listener>(this);
            if (!listener->Run(endpoint.ip, endpoint.port, endpoint.reuse_address)) {
                logger_->Error(LOG_CTX, "Listen on %s:%hu failed", endpoint.ip.c_str(), endpoint.port);
                listeners_.clear();
                return false;
            }
            listeners_.push_back(listener);
        }
        for (size_t i = 0; i < config_.endpoints().size(); ++i) {
            auto& endpoint = config_.endpoints()[i];
            auto& listener = listeners_[i];
            endpoint.port = listener->acceptor().local_endpoint().port();
        }

        is_running_ = true;
        should_stop_ = false;
    }

    /* 启动最低数量的工作线程 */
    NewWorkerThreads(config_.min_num_threads());

    /* 启动管理者线程 */
    std::thread t([this] {
        this->ThreadFunc_Manager();
    });
    t.detach();

    logger_->Info(LOG_CTX, "HttpServer started!");
    return true;
}

/**
 * @brief 停止服务器.
 */
void HttpServer::Stop() {
    StopAsync();
    WaitForStop();
}

/**
 * @brief 停止服务器(异步).
 */
void HttpServer::StopAsync() {
    if (is_running_ && !should_stop_) {
        should_stop_ = true;
        logger_->Info(LOG_CTX, "Waiting for all worker threads to exit ...");
        ioc_->stop();
    }
}

/**
 * @brief 等待服务器停止.
 */
void HttpServer::WaitForStop() const {
    while (!Stopped()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

/**
 * @brief 判断服务器是否已停止.
 */
bool HttpServer::Stopped() const {
    return !is_running_ && ioc_->stopped();
}

/**
 * @brief 创建快照.
 */
SnapshotResult HttpServer::CreateSnapshot() {
    SnapshotResult snapshot;
    snapshot.create_time = std::chrono::system_clock::now();
    snapshot.curr_num_sessions = curr_num_sessions_;
    snapshot.curr_num_worker_threads = curr_num_worker_threads_;
    snapshot.total_num_sessions = total_num_sessions_;
    snapshot.total_num_requests = total_num_requests_;
    {
        std::lock_guard<std::mutex> lck(mutex_requests_);
        SnapshotResult::RequestInfo req_info;
        snapshot.handling_request.reserve(handling_requests_.size());
        for (Request* req : handling_requests_) {
            req_info.id = req->id();
            req_info.method = req->method();
            req_info.content_type = req->content_type();
            req_info.route = req->route();
            req_info.arrive_timepoint = req->arrive_timepoint();
            req_info.path = req->path();
            req_info.client_ip = req->client_ip();
            req_info.client_real_ip = req->client_real_ip();
            snapshot.handling_request.push_back(req_info);
        }
    }
    return snapshot;
}

/**
 * @brief 创建新的工作线程.
 */
void HttpServer::NewWorkerThreads(uint32_t n) {
    for (unsigned int i = 0; i < n; ++i) {
        ++curr_num_worker_threads_;
        std::thread t([this] {
            this->ThreadFunc_Worker();
        });
        t.detach();
    }
}

/**
 * @brief 工作线程.
 */
void HttpServer::ThreadFunc_Worker() {
    const size_t tid = util::thread_id();
    logger_->Debug(LOG_CTX, "Worker thread start. (id=%" PRIu64 ") (sessions:%u, threads:%u)", (uint64_t)tid, (uint32_t)curr_num_sessions_, (uint32_t)curr_num_worker_threads_);

    /* 上次活跃时间 */
    auto last_active_time = std::chrono::steady_clock::now();

    while (true) {
        size_t n = ioc_->run_for(std::chrono::milliseconds(1000));
        {
            std::lock_guard<std::mutex> lck(mutex_server_state_);
            if (should_stop_) {
                --curr_num_worker_threads_;
                break;
            }

            if (n > 0) {
                last_active_time = std::chrono::steady_clock::now();
            }
            else if (curr_num_worker_threads_ > config_.min_num_threads() && curr_num_worker_threads_ > curr_num_sessions_) {
                /* 超过一定时间未活跃，结束当前线程 */
                int64_t dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - last_active_time).count();
                if (dur > 5000) {
                    --curr_num_worker_threads_;
                    break;
                }
            }
        }
    } // end while

    logger_->Debug(LOG_CTX, "Worker thread exit. (id=%" PRIu64 ") (sessions:%u, threads:%u)", (uint64_t)tid, (uint32_t)curr_num_sessions_, (uint32_t)curr_num_worker_threads_);
}

/**
 * @brief 管理者线程.
 */
void HttpServer::ThreadFunc_Manager() {
    while (true) {
        {
            std::lock_guard<std::mutex> lck(mutex_server_state_);
            if (should_stop_) {
                if (curr_num_worker_threads_ == 0) {
                    break;
                }
            }
            else {
                if (curr_num_sessions_ > curr_num_worker_threads_ && curr_num_worker_threads_ < config_.max_num_threads()) {  /* 触发扩容 */
                    /* 扩容为1.5倍, 单次最多32个线程, 且扩容后总线程数量不能超过最大限制 */
                    unsigned int inc_num_threads = s_clamp(curr_num_worker_threads_ / 2, 1U, std::min(32U, config_.max_num_threads() - curr_num_worker_threads_));
                    NewWorkerThreads(inc_num_threads);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    is_running_ = false;
}

/**
 * @brief 创建新的Session后回调.
 */
void HttpServer::OnNewSession() {
    ++curr_num_sessions_;
    ++total_num_sessions_;
}

/**
 * @brief 销毁Session后回调.
 */
void HttpServer::OnDestroySession() {
    --curr_num_sessions_;
}

/**
 * @brief 开始处理请求回调.
 */
void HttpServer::OnStartRequest(Request* req) {
    ++total_num_requests_;
    std::lock_guard<std::mutex> lck(mutex_requests_);
    handling_requests_.emplace(req);
}

/**
 * @brief 请求处理完成回调.
 */
void HttpServer::OnFinishRequest(Request* req) {
    std::lock_guard<std::mutex> lck(mutex_requests_);
    handling_requests_.erase(req);
}

/**
 * @brief 获取二进制程序所在目录，绝对路径，以'/'结尾.
 */
const std::string& HttpServer::GetBinDir() {
    return util::path::get_bin_dir();
}

/**
 * @brief 获取二进制程序所在目录，绝对路径，以'/'结尾，UTF8编码.
 */
const std::string& HttpServer::GetBinDirUtf8() {
    return util::path::get_bin_dir_utf8();
}

} // namespace server
} // namespace ic
