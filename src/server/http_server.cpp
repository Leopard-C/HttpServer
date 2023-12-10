#include "server/http_server.h"
#include "server/request.h"
#include "server/router.h"
#include "server/util/format_time.h"
#include "server/util/hash/xxh64.h"
#include "server/util/path.h"
#include "server/util/thread.h"
#include "listener.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <log/logger.h>

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
    active(false), count(0), request_id(-1), path_id(0), thread_id(util::thread_id()),
    start(s_time0), finish(s_time0), svr_(svr)
{
}

Json::Value ThreadInfo::ToJson(tp now/* = std::chrono::system_clock::now()*/) const {
    auto route = svr_->router()->GetRoute(path_id);
    Json::Value node;
    node["self"] = (this->thread_id == util::thread_id());  /* 是否是当前请求 */
    node["thread_id"] = thread_id;
    node["path_id"] = std::to_string(path_id);
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

HttpServer::HttpServer(const HttpServerConfig& config) :
    config_(config), listener_(),
    ioc_(std::make_shared<net::io_context>(config.num_threads())),
    router_(std::make_shared<Router>(this))
{
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

uint64_t HttpServer::XXH64(const std::string& str) const {
    return util::hash::xxh64(str, config_.xxh64_seed());
}

bool HttpServer::Listen() {
    beast::error_code ec;
    const auto address = net::ip::make_address(config_.ip(), ec);
    if (ec) {
        LError("make_address failed: {}", ec.message());
        LError("Listen on {}:{} failed", config_.ip(), config_.port());
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
        LError("HttpServerConfig.num_threads({}) shoule be in range: [1, {}]", num_threads, max_num_threads);
        return;
    }

    LInfo("HttpServer started!");

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
        LInfo("Waiting for thread [{}/{}] ...", i+2, num_threads);
        worker_threads_[i].join();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    LInfo("HttpServer stopped!");
}

void HttpServer::RunIoContext() {
    size_t tid = util::thread_id();
    {
        std::lock_guard<std::mutex> lck(mutex_for_thread_infos_);
        thread_infos_.emplace(tid, new ThreadInfo(this));
    }
    LInfo("ioc run, thread id: {}", tid);
    ioc_->run();
    LInfo("ioc stop, thread id: {}", tid);
}

/**
 * @brief 停止服务器（非阻塞）.
 */
void HttpServer::Stop() {
    LInfo("Waiting for all threads to exit ...");
    LInfo("Waiting for thread [{}/{}] ...", 1, config_.num_threads());
    //LInfo("thread infos:\n{}", DumpThreadInfos().toStyledString());
    LInfo("ioc is stopping ...");
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
        ti->request_id = req.id();
        ti->path_id = req.path_id();
        ti->active = true;
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
