/**
 * @file router.h
 * @brief 路由.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_SERVER_ROUTER_H_
#define IC_SERVER_ROUTER_H_
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <jsoncpp/json/value.h>
#include "http_method.h"

/**
 * @brief 是否使用`Boost.shared_mutex`.
 * @note `std::shared_mutex`是`C++17`才引入的
 */
#define IC_SERVER_USE_BOOST_SHARED_MUTEX 1

#if IC_SERVER_USE_BOOST_SHARED_MUTEX == 1
#  include <boost/thread/lock_types.hpp>
#  include <boost/thread/shared_mutex.hpp>
#else
#  include <shared_mutex>
#endif

/**
 * @brief 是否使用`Boost.Regex`.
 * @note 使用`Boost.Regex`正则表达式库，比`std::regex`性能更好
 */
#define IC_SERVER_USE_BOOST_REGEX 1

#if IC_SERVER_USE_BOOST_REGEX == 1
#  include <boost/regex.hpp>
#  define REGEX_NAMESPACE boost
#else
#  include <regex>
#  define REGEX_NAMESPACE std
#endif

namespace ic {
namespace server {

class Request;
class Response;
class HttpServer;

/**
 * @brief 路由.
 */
class Route {
public:
    using ResponseCallback = std::function<void(Request&, Response&)>;
    using ResponseJsonCallback = std::function<void(Request&, Json::Value&)>;

public:
    Route(const std::string& path, int methods, ResponseCallback cb, const std::string& desc, const std::unordered_map<std::string, std::string>& cfg)
        : path(path), description(desc), methods(methods), response_callback_(cb), configuration(cfg) {}
    Route(const std::string& path, int methods, ResponseJsonCallback cb, const std::string& desc, const std::unordered_map<std::string, std::string>& cfg)
        : path(path), description(desc), methods(methods), response_json_callback_(cb), configuration(cfg) {}
    virtual ~Route() = default;

    virtual bool is_static() const = 0;
    bool is_regex() const { return !is_static(); }
    std::string GetMethodsString() const;
    Json::Value ToJson() const;
    void Invoke(Request& req, Response& res) const;

public:
    /** 支持的HTTP请求方法(如果支持多种方法，使用或运算，如 HttpMethod::kGET | HttpMethod::kPOST) */
    int methods = HttpMethod::kNotSupport;

    /** 程序运行期间命中该路由的次数 */
    std::atomic_uint64_t hit_count{0};

    /** 路径 */
    std::string path;

    /** 描述信息 */
    std::string description;

    /** 配置信息(可以用于请求拦截器中，如是否需要鉴权) */
    std::unordered_map<std::string, std::string> configuration;

private:
    ResponseCallback response_callback_;
    ResponseJsonCallback response_json_callback_;
};

/**
 * @brief 静态路由.
 * @example /web/index.html
 * @example /User/GetUserInfo
 */
class StaticRoute : public Route {
public:
    StaticRoute(const std::string& path, int methods, ResponseCallback cb, const std::string& desc, const std::unordered_map<std::string, std::string>& cfg)
        : Route(path, methods, cb, desc, cfg) {}
    StaticRoute(const std::string& path, int methods, ResponseJsonCallback cb, const std::string& desc, const std::unordered_map<std::string, std::string>& cfg)
        : Route(path, methods, cb, desc, cfg) {}
    virtual bool is_static() const override { return true; }
};

/**
 * @brief 正则表达式路由.
 * @example /web/img/(.+)
 * @example /User/Space/(\d+)
 */
class RegexRoute : public Route {
public:
    RegexRoute(const std::string& path, int methods, ResponseCallback cb, const std::string& desc, const std::unordered_map<std::string, std::string>& cfg, int priority = 0)
        : Route(path, methods, cb, desc, cfg), regex(path), priority(priority) {}
    RegexRoute(const std::string& path, int methods, ResponseJsonCallback cb, const std::string& desc, const std::unordered_map<std::string, std::string>& cfg, int priority = 0)
        : Route(path, methods, cb, desc, cfg), regex(path), priority(priority) {}
    virtual bool is_static() const override { return false; }
    /** 匹配优先级(值越大，越优先匹配) */
    int priority;
    /** 正则表达式 */
    REGEX_NAMESPACE::regex regex;
};

using RoutePtr = std::shared_ptr<Route>;
using StaticRoutePtr = std::shared_ptr<StaticRoute>;
using RegexRoutePtr = std::shared_ptr<RegexRoute>;

using RouteConstPtr = std::shared_ptr<const Route>;
using StaticRouteConstPtr = std::shared_ptr<const StaticRoute>;
using RegexRouteConstPtr = std::shared_ptr<const RegexRoute>;

/**
 * @brief 路由管理器.
 * 
 * @note 服务器运行过程中，可以动态新增或删除路由，且线程安全.
 */
class Router {
public:
    Router(HttpServer* svr);
    ~Router();

    /**
     * @brief 添加静态路由，如果已存在则覆盖.
     */
    bool AddStaticRoute(StaticRoutePtr route);
    bool AddStaticRoute(const std::string& path, int methods,
        std::function<void(Request&, Response&)> callback,
        const std::string& description = "",
        const std::unordered_map<std::string, std::string>& configuration = {});
    bool AddStaticRoute(const std::string& path, int methods,
        std::function<void(Request&, Json::Value&)> callback,
        const std::string& description = "",
        const std::unordered_map<std::string, std::string>& configuration = {});

    /**
     * @brief 添加正则路由，如果已存在则覆盖.
     */
    bool AddRegexRoute(RegexRoutePtr route);
    bool AddRegexRoute(const std::string& path, int methods, std::function<void(Request&, Response&)> callback,
        const std::string& description = "", const std::unordered_map<std::string, std::string>& configuration = {}, int priority = 0);
    bool AddRegexRoute(const std::string& path, int methods, std::function<void(Request&, Json::Value&)> callback,
        const std::string& description = "", const std::unordered_map<std::string, std::string>& configuration = {}, int priority = 0);

    /**
     * @brief 删除路由.
     */
    void DeleteRoute(const std::string& path);

    /**
     * @brief 检查请求是否命中已注册的路由.
     */
    bool HitRoute(Request& req, Response& res);

    /**
     * @brief 获取路由.
     */
    RouteConstPtr GetRoute(const std::string& path);

    std::unordered_map<std::string, RouteConstPtr> routes();
    std::unordered_map<std::string, StaticRouteConstPtr> static_routes();
    std::vector<RegexRouteConstPtr> regex_routes();

    const HttpServer* svr() const { return svr_; }

    void set_cb_invalid_path(Route::ResponseCallback cb);
    void set_cb_invalid_method(Route::ResponseCallback cb);

private:
    /**
     * @brief 检查路由是否有效.
     */
    bool CheckRoute(RoutePtr route) const;

    /**
     * @brief 删除路由(无锁).
     */
    void DeleteRoute_WithoutLock(const std::string& path);

private:
    HttpServer* svr_;

#if IC_SERVER_USE_BOOST_SHARED_MUTEX == 1
    using SharedMutex = boost::shared_mutex;
    using ReadLock = boost::shared_lock<boost::shared_mutex>;
    using WriteLock = boost::unique_lock<boost::shared_mutex>;
#else
    using SharedMutex = std::shared_mutex;
    using ReadLock = std::shared_lock<std::shared_mutex>;
    using WriteLock = std::unique_lock<std::shared_mutex>;
#endif

    /** 读写锁 */
    SharedMutex mutex_;

    /** 所有路由 */
    std::unordered_map<std::string, RoutePtr> routes_;

    /** 静态路由 */
    std::unordered_map<std::string, StaticRoutePtr> static_routes_;

    /** 正则表达式路由 */
    std::vector<RegexRoutePtr> regex_routes_;

    /** 路径无效的回调函数 */
    Route::ResponseCallback cb_invalid_path_;

    /** 请求方法无效的回调函数 */
    Route::ResponseCallback cb_invalid_method_;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_ROUTER_H_
