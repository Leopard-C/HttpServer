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
#include <map>
#include <string>
#include <jsoncpp/json/value.h>
#include "http_method.h"

/**
 * @note 使用Boost.Regex正则表达式库，比标准库的regex性能更好
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
    Route(const std::string& path, int methods, ResponseCallback cb, const std::string& desc, const std::map<std::string, std::string>& cfg)
        : path(path), description(desc), methods(methods), response_callback_(cb), configuration(cfg) {}
    Route(const std::string& path, int methods, ResponseJsonCallback cb, const std::string& desc, const std::map<std::string, std::string>& cfg)
        : path(path), description(desc), methods(methods), response_json_callback_(cb), configuration(cfg) {}
    virtual ~Route() = default;

    virtual bool is_static() const = 0;
    bool is_regex() const { return !is_static(); }
    std::string GetMethodsString() const;
    void Invoke(Request& req, Response& res) const;

public:
    /** 支持的HTTP请求方法(如果支持多种方法，使用或运算，如 HttpMethod::kGET | HttpMethod::kPOST) */
    int methods = HttpMethod::kNotSupport;

    /** 路由ID */
    uint64_t id;

    /** 程序运行期间命中该路由的次数 */
    std::atomic_uint64_t hit_count{0};

    /** 路径 */
    std::string path;

    /** 描述信息 */
    std::string description;

    /** 配置信息(可以用于请求拦截器中，如是否需要鉴权) */
    std::map<std::string, std::string> configuration;

private:
    ResponseCallback response_callback_;
    ResponseJsonCallback response_json_callback_;
};

/**
 * @brief 静态路由.
 */
class StaticRoute : public Route {
public:
    StaticRoute(const std::string& path, int methods, ResponseCallback cb, const std::string& desc, const std::map<std::string, std::string>& cfg)
        : Route(path, methods, cb, desc, cfg) {}
    StaticRoute(const std::string& path, int methods, ResponseJsonCallback cb, const std::string& desc, const std::map<std::string, std::string>& cfg)
        : Route(path, methods, cb, desc, cfg) {}
    virtual bool is_static() const override { return true; }
};

/**
 * @brief 正则表达式路由.
 */
class RegexRoute : public Route {
public:
    RegexRoute(const std::string& path, int methods, ResponseCallback cb, const std::string& desc, const std::map<std::string, std::string>& cfg)
        : Route(path, methods, cb, desc, cfg), regex(path) {}
    RegexRoute(const std::string& path, int methods, ResponseJsonCallback cb, const std::string& desc, const std::map<std::string, std::string>& cfg)
        : Route(path, methods, cb, desc, cfg), regex(path) {}
    virtual bool is_static() const override { return false; }
    REGEX_NAMESPACE::regex regex;
};

/**
 * @brief 路由管理器.
 * 
 * @warning 服务器(HttpServer)启动后，不能再修改路由
 */
class Router {
public:
    Router(HttpServer* svr);
    ~Router();

    bool AddStaticRoute(StaticRoute* route);
    bool AddStaticRoute(const std::string& path, int methods,
        std::function<void(Request&, Response&)> callback,
        const std::string& description = "",
        const std::map<std::string, std::string>& configuration = {});
    bool AddStaticRoute(const std::string& path, int methods,
        std::function<void(Request&, Json::Value&)> callback,
        const std::string& description = "",
        const std::map<std::string, std::string>& configuration = {});

    bool AddRegexRoute(RegexRoute* route);
    bool AddRegexRoute(const std::string& path, int methods,
        std::function<void(Request&, Response&)> callback,
        const std::string& description = "",
        const std::map<std::string, std::string>& configuration = {});
    bool AddRegexRoute(const std::string& path, int methods,
        std::function<void(Request&, Json::Value&)> callback,
        const std::string& description = "",
        const std::map<std::string, std::string>& configuration = {});

    void set_cb_invalid_path(Route::ResponseCallback cb);
    void set_cb_invalid_method(Route::ResponseCallback cb);

    const HttpServer* svr() const { return svr_; }
    const std::map<uint64_t, Route*>& routes() const { return routes_; }
    const std::map<uint64_t, StaticRoute*>& static_routes() const { return static_routes_; }
    const std::vector<RegexRoute*>& regex_routes() const { return regex_routes_; }

    /**
     * @brief 检查请求是否命中已注册的路由.
     */
    bool HitRoute(Request& req, Response& res) const;

    /**
     * @brief 获取路由.
     */
    const Route* GetRoute(uint64_t path_id) const;

private:
    bool CheckRoute(Route* route) const;

private:
    HttpServer* svr_{nullptr};

    /** 所有路由 */
    std::map<uint64_t, Route*> routes_;

    /** 静态路由 */
    std::map<uint64_t, StaticRoute*> static_routes_;

    /** 正则表达式路由 */
    std::vector<RegexRoute*> regex_routes_;

    /** 路径无效的回调函数 */
    Route::ResponseCallback cb_invalid_path_;

    /** 请求方法无效的回调函数 */
    Route::ResponseCallback cb_invalid_method_;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_ROUTER_H_
