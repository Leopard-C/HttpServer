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
        v_handling_request["thread_id"] = request.thread_id;
        v_handling_request["id"] = request.id;
        v_handling_request["method"] = to_string(request.method);
        v_handling_request["content_type"] = request.content_type.ToString();
        v_handling_request["arrive_timepoint"] = util::format_time_us(request.arrive_timepoint);
        v_handling_request["path"] = request.path;
        v_handling_request["route"] = request.route->ToJson();
        v_handling_request["client_ip"] = request.client_ip;
        v_handling_request["client_real_ip"] = request.client_real_ip;
        /* 是否是当前请求 */
        v_handling_request["is_self"] = (util::thread_id() == request.thread_id);
        /* 已经耗时(秒) */
        v_handling_request["elapsed_seconds"] = std::chrono::duration_cast<std::chrono::microseconds>(create_time - request.arrive_timepoint).count() / 1000000.0;
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
    config_(config), logger_(logger)
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
        auto& endpoints = config_.endpoints();
        for (size_t i = 0; i < endpoints.size(); ++i) {
            auto listener = std::make_shared<Listener>(this);
            if (!listener->Run(endpoints[i].ip, endpoints[i].port, endpoints[i].reuse_address)) {
                logger_->Error(LOG_CTX, "Listener start failed");
                listeners_.clear();
                return false;
            }
            listeners_.push_back(listener);
        }
        for (size_t i = 0; i < endpoints.size(); ++i) {
            /* 如果配置的端口为0，会任意选择一个可用端口，需要获取到该端口 */
            if (endpoints[i].port == 0) {
                endpoints[i].port = listeners_[i]->acceptor().local_endpoint().port();
            }
        }

        is_running_ = true;
        should_stop_ = false;
    }

    logger_->Info(LOG_CTX, "HttpServer started!");

    /* 启动最低数量的工作线程 */
    NewWorkerThreads(config_.min_num_threads());

    /* 启动管理者线程 */
    std::thread t([this] {
        this->ThreadFunc_Manager();
    });
    t.detach();

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
    std::lock_guard<std::mutex> lck(mutex_server_state_);
    if (is_running_) {
        logger_->Info(LOG_CTX, "Waiting for %u worker threads to exit ...", (uint32_t)curr_num_worker_threads_);
        should_stop_ = true;
        ioc_->stop();
    }
}

/**
 * @brief 等待服务器停止.
 */
void HttpServer::WaitForStop() {
    {
        /* 禁止在工作线程中调用该函数，否则服务器永远无法退出 */
        std::lock_guard<std::mutex> lck(mutex_server_state_);
        if (worker_thread_ids_.find(util::thread_id()) != worker_thread_ids_.end()) {
            logger_->Warn(LOG_CTX,
                "DO NOT CALL HttpServer::Stop() or HttpServer::WaitForStop() in worker thread(id=%" PRIu64 "). "
                "Use HttpServer::StopAsync() instead.",
                (uint64_t)util::thread_id()
            );
            return;
        }
    }
    while (!Stopped()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

/**
 * @brief 判断服务器是否已停止.
 */
bool HttpServer::Stopped() const {
    return !is_running_;
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
            req_info.thread_id = req->thread_id();
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
    std::sort(snapshot.handling_request.begin(), snapshot.handling_request.end(), [](SnapshotResult::RequestInfo& a, SnapshotResult::RequestInfo& b) {
        return a.arrive_timepoint < b.arrive_timepoint;
    });
    return snapshot;
}

/**
 * @brief 创建新的工作线程.
 * @note 调用该函数前，已经对`mutex_server_state_`进行加锁.
 */
void HttpServer::NewWorkerThreads(uint32_t n) {
    logger_->Debug(LOG_CTX, "Starting %u worker threads. (sessions:%u, threads:%u)", n, (uint32_t)curr_num_sessions_, (uint32_t)curr_num_worker_threads_ + n);
    for (uint32_t i = 0; i < n; ++i) {
        ++curr_num_worker_threads_;
        std::thread t([this] {
            this->ThreadFunc_Worker();
            if (this->cb_before_worker_thread_exit_) {
                this->cb_before_worker_thread_exit_();
            }
        });
        t.detach();
    }
}

/**
 * @brief 工作线程.
 */
void HttpServer::ThreadFunc_Worker() {
    const size_t tid = util::thread_id();
    logger_->Debug(LOG_CTX, "Worker thread start. (id=%" PRIu64 ")", (uint64_t)tid);
    {
        std::lock_guard<std::mutex> lck(mutex_server_state_);
        worker_thread_ids_.emplace(tid);
    }

    /* 上次活跃时间 */
    auto last_active_time = std::chrono::steady_clock::now();
    /* 是否需要退出当前线程 */
    bool exit = false;

    while (true) {
        size_t n = ioc_->run_for(std::chrono::milliseconds(1000));

        std::lock_guard<std::mutex> lck(mutex_server_state_);
        if (should_stop_) {
            exit = true;
        }
        else if (n > 0) {
            last_active_time = std::chrono::steady_clock::now();
        }
        else if (curr_num_worker_threads_ > config_.min_num_threads() && curr_num_worker_threads_ > curr_num_sessions_) {
            /* 超过一定时间未活跃，结束当前线程 */
            int64_t dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - last_active_time).count();
            if (dur > 5000) {
                exit = true;
            }
        }

        if (exit) {
            logger_->Debug(LOG_CTX, "Worker thread exit. (id=%" PRIu64 ") (sessions:%u, threads:%u)", (uint64_t)tid, (uint32_t)curr_num_sessions_, (uint32_t)curr_num_worker_threads_);
            --curr_num_worker_threads_;
            worker_thread_ids_.erase(tid);
            break;
        }
    } // end while
}

/**
 * @brief 管理者线程.
 */
void HttpServer::ThreadFunc_Manager() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::lock_guard<std::mutex> lck(mutex_server_state_);
        if (should_stop_) {
            if (curr_num_worker_threads_ == 0) {
                logger_->Info(LOG_CTX, "HttpServer stopped!");
                is_running_ = false;
                break;
            }
        }
        else if ((curr_num_sessions_ > curr_num_worker_threads_ || curr_num_handling_requests_ == curr_num_worker_threads_) &&
                 curr_num_worker_threads_ < config_.max_num_threads())
        {
            /* 扩容为1.5倍, 单次最多32个线程, 且扩容后总线程数量不能超过最大限制 */
            uint32_t inc_num_threads = s_clamp(curr_num_worker_threads_ / 2U, 1U, std::min(32U, config_.max_num_threads() - curr_num_worker_threads_));
            NewWorkerThreads(inc_num_threads);
            if (curr_num_worker_threads_ == config_.max_num_threads()) {
                logger_->Debug(LOG_CTX, "Number of worker threads has reached the peak(%u)", config_.max_num_threads());
            }
        }
    } // end while
}

void HttpServer::OnNewSession() {
    ++curr_num_sessions_;
    ++total_num_sessions_;
}

void HttpServer::OnDestroySession() {
    --curr_num_sessions_;
}

void HttpServer::OnStartHandlingRequest(Request* req) {
    std::lock_guard<std::mutex> lck(mutex_requests_);
    handling_requests_.emplace(req);
    ++curr_num_handling_requests_;
    ++total_num_requests_;
}

void HttpServer::OnFinishHandlingRequest(Request* req) {
    std::lock_guard<std::mutex> lck(mutex_requests_);
    handling_requests_.erase(req);
    --curr_num_handling_requests_;
}

} // namespace server
} // namespace ic
