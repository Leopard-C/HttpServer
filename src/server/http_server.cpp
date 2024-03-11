#include "server/http_server.h"
#include "server/logger.h"
#include "server/request.h"
#include "server/request_raw.h"
#include "server/router.h"
#include "server/util/format_time.h"
#include "server/util/path.h"
#include "server/util/thread.h"
#include "listener.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace ic {
namespace server {

/**
 * @brief time_t表示的时间零点，可以用来表示一个无效的时间点.
 */
static const std::chrono::system_clock::time_point s_time0 = std::chrono::system_clock::from_time_t(0);

/**
 * @brief 二进制程序所在目录.
 */
static const std::string s_bin_dir = util::path::get_bin_dir();


/*******************************************************************
**
**                          ThreadInfo
** 
*******************************************************************/

ThreadInfo::ThreadInfo(HttpServer* svr) :
    active(false), count(0), request_id(-1), route(nullptr), thread_id(util::thread_id()),
    start(s_time0), finish(s_time0), svr_(svr)
{
}

Json::Value ThreadInfo::ToJson(tp now/* = std::chrono::system_clock::now()*/) const {
    Json::Value node;
    node["self"] = (this->thread_id == util::thread_id());  /* 是否是当前请求 */
    node["thread_id"] = thread_id;
    node["path"] = route ? route->path : std::string();
    node["count"] = count;
    node["active"] = active;
    node["start"] = util::format_time_us(start);
    node["finish"] = util::format_time_us(finish);
    node["start_tp"] = uint64_t(start.time_since_epoch().count()) / 1000;  /* 微秒时间戳 */
    node["finish_tp"] = uint64_t(finish.time_since_epoch().count()) / 1000;
    std::chrono::nanoseconds duration{0};
    if (start != s_time0) {
        duration = (finish == s_time0 ? now : finish) - start;
    }
    node["duration"] = util::format_duration(duration);
    return node;
}


/*******************************************************************
**
**                          HttpServer
** 
*******************************************************************/

HttpServer::HttpServer(const HttpServerConfig& config, std::shared_ptr<ILogger> logger/* = nullptr*/) :
    config_(config), logger_(logger), listener_(),
    ioc_(std::make_shared<net::io_context>(config.num_threads())),
    router_(std::make_shared<Router>(this))
{
    if (!logger_) {
        logger_ = std::make_shared<ConsoleLogger>(LogLevel::kInfo, LogLevel::kWarn);
    }
}

HttpServer::~HttpServer() {
    for (auto iter = thread_infos_.begin(); iter != thread_infos_.end(); ++iter) {
        delete iter->second;
        iter->second = nullptr;
    }
}

std::map<size_t, ThreadInfo> HttpServer::thread_infos() {
    std::map<size_t, ThreadInfo> thread_infos;
    {
        std::lock_guard<std::mutex> lck(mutex_for_thread_infos_);
        for (auto iter = thread_infos_.begin(); iter != thread_infos_.end(); ++iter) {
            thread_infos.emplace(iter->first, *(iter->second));
        }
    }
    return thread_infos;
}

Json::Value HttpServer::DumpThreadInfos() {
    auto now = std::chrono::system_clock::now();
    auto s = thread_infos();
    Json::Value data;
    for (auto iter = s.begin(); iter != s.end(); ++iter) {
        data.append(iter->second.ToJson(now));
    }
    return data;
}

bool HttpServer::Listen() {
    beast::error_code ec;
    const auto address = net::ip::make_address(config_.ip(), ec);
    if (ec) {
        logger_->Error(LOG_CTX, "Make_address failed: %s", ec.message().c_str());
        logger_->Error(LOG_CTX, "Listen on %s:%u failed", config_.ip().c_str(), config_.port());
        return false;
    }
    listener_ = std::make_shared<Listener>(this, tcp::endpoint{ address, (unsigned short)config_.port() });
    return listener_->Run();
}

/**
 * @brief 启动服务器（阻塞当前线程）.
 */
void HttpServer::Start() {
    unsigned int num_threads = config_.num_threads();
    unsigned int max_num_threads = std::thread::hardware_concurrency() * 20;
    if (num_threads < 1 || num_threads > max_num_threads) {
        logger_->Error(LOG_CTX, "HttpServerConfig.num_threads(%u) shoule be in range: [1, %u]", num_threads, max_num_threads);
        return;
    }

    logger_->Info(LOG_CTX, "HttpServer started!");

    /* 启动所有线程 */
    worker_threads_.clear();
    worker_threads_.reserve(num_threads - 1);
    for (unsigned int i = 0; i < num_threads - 1; ++i) {
        worker_threads_.emplace_back([this]{
            RunIoContext();
        });
    }
    RunIoContext();

    /* 等待所有线程退出 */
    for (unsigned int i = 0; i < num_threads - 1; ++i) {
        logger_->Info(LOG_CTX, "Waiting for thread [%u/%u] ...", i+2, num_threads);
        worker_threads_[i].join();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    logger_->Info(LOG_CTX, "HttpServer stopped!");
}

void HttpServer::RunIoContext() {
    size_t tid = util::thread_id();
    {
        std::lock_guard<std::mutex> lck(mutex_for_thread_infos_);
        thread_infos_.emplace(tid, new ThreadInfo(this));
    }
    logger_->Info(LOG_CTX, "ioc run, thread id: %" PRIu64, (uint64_t)tid);
    ioc_->run();
    logger_->Info(LOG_CTX, "ioc stop, thread id: %" PRIu64, (uint64_t)tid);
}

/**
 * @brief 停止服务器（非阻塞）.
 */
void HttpServer::Stop() {
    logger_->Info(LOG_CTX, "Waiting for all threads to exit ...");
    logger_->Info(LOG_CTX, "Waiting for thread [%u/%u] ...", 1, config_.num_threads());
    //logger_->Info(LOG_CTX, "thread infos:\n%s", DumpThreadInfos().toStyledString().c_str());
    logger_->Info(LOG_CTX, "ioc is stopping ...");
    std::thread t([this]{
        if (!ioc_->stopped()) {
            ioc_->stop();
        }
    });
    t.detach();
}

/**
 * @brief 判断服务器是否已停止.
 */
bool HttpServer::Stopped() const {
    return ioc_->stopped();
}

/**
 * @brief 根据线程ID获取线程信息.
 */
ThreadInfo* HttpServer::GetThreadInfo(size_t tid) {
    std::lock_guard<std::mutex> lck(mutex_for_thread_infos_);
    auto iter = thread_infos_.find(tid);
    if (iter != thread_infos_.end()) {
        return iter->second;
    }
    return nullptr;
}

/**
 * @brief 开始处理请求.
 */
void HttpServer::StartRequest(Request& req) {
    static thread_local ThreadInfo* ti = GetThreadInfo(util::thread_id());
    if (ti) {
        ti->active = true;
        ti->request_id = req.id();
        ti->route = req.route();
        ti->start = req.arrive_timepoint();
        ti->finish = s_time0;
        ti->count++;
        req.thread_info_ = ti;
    }
}

/**
 * @brief 请求处理完成.
 */
void HttpServer::FinishRequest(Request& req) {
    static thread_local ThreadInfo* ti = GetThreadInfo(util::thread_id());
    if (ti) {
        ti->active = false;
        ti->finish = std::chrono::system_clock::now();
        req.time_consumed_total_ = std::chrono::duration_cast<std::chrono::nanoseconds>(ti->finish - ti->start);
    }
}

/**
 * @brief 获取二进制程序所在目录，绝对路径，以'/'结尾.
 */
const std::string& HttpServer::GetBinDir() {
    return s_bin_dir;
}

} // namespace server
} // namespace ic
