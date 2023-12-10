#include "server/router.h"
#include "server/http_server.h"
#include "server/request.h"
#include "server/response.h"
#include "server/status/base.h"
#include <log/logger.h>

namespace ic {
namespace server {

std::string Route::GetMethodsString() const {
    static const HttpMethod methods_arr[] = {
        HttpMethod::kGET, HttpMethod::kHEAD, HttpMethod::kPOST, HttpMethod::kPUT, HttpMethod::kDELETE,
        HttpMethod::kCONNECT, HttpMethod::kOPTIONS, HttpMethod::kTRACE, HttpMethod::kPATCH
    };
    std::string str;
    for (HttpMethod method : methods_arr) {
        if (this->methods & (int)method) {
            if (str.length()) {
                str += ',';
            }
            str += to_string(method);
        }
    }
    if (str.empty()) {
        str = "NOTSUPPORT";
    }
    return str;
}

void Route::Invoke(Request& req, Response& res) const {
    if (response_callback_) {
        response_callback_(req, res);
    }
    else if (response_json_callback_) {
        Json::Value root;
        response_json_callback_(req, root);
        res.SetJsonBody(root);
    }
}

Router::Router(HttpServer* svr) : svr_(svr) {
    cb_invalid_path_ = [](Request& req, Response& res) {
        Json::Value root;
        root["code"] = status::base::kInvalidPath;
        root["msg"] = "Invalid request path";
        res.SetJsonBody(root);
    };
    cb_invalid_method_ = [](Request& req, Response& res) {
        Json::Value root;
        root["code"] = status::base::kInvalidMethod;
        root["msg"] = "Invalid http method";
        res.SetJsonBody(root);
    };
}

Router::~Router() {
    for (auto iter = routes_.begin(); iter != routes_.end(); ++iter) {
        delete iter->second;
        iter->second = nullptr;
    }
}

/**
 * @brief 添加静态路由.
 */
bool Router::AddStaticRoute(StaticRoute* route) {
    if (!CheckRoute(route)) {
        return false;
    }
    LDebug("Add static route: {:>4} {}", route->GetMethodsString(), route->path);
    routes_.emplace(route->id, route);
    static_routes_.emplace(route->id, route);
    return true;
}

bool Router::AddStaticRoute(const std::string& path, int methods,
    std::function<void(Request&, Response&)> callback,
    const std::string& description/* = ""*/,
    const std::map<std::string, std::string>& configuration/* = {}*/)
{
    StaticRoute* route = new StaticRoute(path, methods, callback, description, configuration);
    if (!AddStaticRoute(route)) {
        delete route;
        return false;
    }
    return true;
}

bool Router::AddStaticRoute(const std::string& path, int methods,
    std::function<void(Request&, Json::Value&)> callback,
    const std::string& description/* = ""*/,
    const std::map<std::string, std::string>& configuration/* = {}*/)
{
    StaticRoute* route = new StaticRoute(path, methods, callback, description, configuration);
    if (!AddStaticRoute(route)) {
        delete route;
        return false;
    }
    return true;
}

/**
 * @brief 添加正则路由.
 */
bool Router::AddRegexRoute(RegexRoute* route) {
    if (!CheckRoute(route)) {
        return false;
    }
    LDebug("Add  regex route: {:>4} {}", route->GetMethodsString(), route->path);
    routes_.emplace(route->id, route);
    regex_routes_.emplace_back(route);
    return true;
}

bool Router::AddRegexRoute(const std::string& path, int methods,
    std::function<void(Request&, Response&)> callback,
    const std::string& description/* = ""*/,
    const std::map<std::string, std::string>& configuration/* = {}*/)
{
    try {
        RegexRoute* route = new RegexRoute(path, methods, callback, description, configuration);
        if (!AddRegexRoute(route)) {
            delete route;
            return false;
        }
        return true;
    }
    catch (const REGEX_NAMESPACE::regex_error& ex) {
        LError("Invalid regex pattern: {}, can not add to router. {}", path, ex.what());
        return false;
    }
}

bool Router::AddRegexRoute(const std::string& path, int methods,
    std::function<void(Request&, Json::Value&)> callback,
    const std::string& description/* = ""*/,
    const std::map<std::string, std::string>& configuration/* = {}*/)
{
    try {
        RegexRoute* route = new RegexRoute(path, methods, callback, description, configuration);
        if (!AddRegexRoute(route)) {
            delete route;
            return false;
        }
        return true;
    }
    catch (const REGEX_NAMESPACE::regex_error& ex) {
        LError("Invalid regex pattern: {}, can not add to router. {}", path, ex.what());
        return false;
    }
}

void Router::set_cb_invalid_path(Route::ResponseCallback cb) {
    if (cb) {
        cb_invalid_path_ = cb;
    }
}

void Router::set_cb_invalid_method(Route::ResponseCallback cb) {
    if (cb) {
        cb_invalid_method_ = cb;
    }
}

/**
 * @brief 检查路由是否合法.
 */
bool Router::CheckRoute(Route* route) const {
    if (route->path.empty()) {
        LError("Empty path, can not add to router");
        return false;
    }
    if (!(route->methods & 0x1ff)) {
        LError("Invalid http methods, can not add to router");
        return false;
    }
    route->id = svr_->XXH64(route->path);
    auto iter = routes_.find(route->id);
    if (iter != routes_.end()) {
        if (iter->second->path == route->path) {
            LError("Duplicate path: {}, can not add to router", route->path);
        }
        else {
            LCritical("Duplicate xxh64 hash value. path1: {}, path2: {}, hash: {}",
                      iter->second->path, route->path, route->id);
        }
        return false;
    }
    return true;
}

/**
 * @brief 获取路由.
 */
const Route* Router::GetRoute(uint64_t path_id) const {
    auto find_iter = routes_.find(path_id);
    if (find_iter != routes_.end()) {
        return find_iter->second;
    }
    return nullptr;
}

/**
 * @brief 请求路径命中的路由，未命中则返回nullptr.
 */
bool Router::HitRoute(Request& req, Response& res) const {
    /* 检查是否有匹配的路由 */
    Route* route = nullptr;
    auto iter = static_routes_.find(req.path_id());
    if (iter != static_routes_.end()) {
        route = iter->second;
    }
    else {
        try {
            REGEX_NAMESPACE::smatch match;
            for (auto r : regex_routes_) {
                if (!REGEX_NAMESPACE::regex_match(req.path(), match, r->regex)) {
                    continue;
                }
                req.route_regex_match_.reserve(match.size() - 1);
                for (size_t i = 1; i < match.size(); ++i) {
#if IC_SERVER_USE_BOOST_REGEX == 1
                    req.route_regex_match_.push_back(match.str(static_cast<int>(i)));
#else
                    req.route_regex_match_.push_back(match.str(i));
#endif
                }
                route = r;
                break;
            }
            if (!route) {
                cb_invalid_path_(req, res);
                return false;
            }
        }
        catch (const std::exception& ex) {
            LError("regex_match error, path: {}, err_msg: {}", req.path(), ex.what());
            cb_invalid_path_(req, res);
            return false;
        }
    }

    /* 检查路由对应的HTTP请求方法是否匹配 */
    if (!(route->methods & (int)req.method())) {
        cb_invalid_method_(req, res);
        return false;
    }

    /* 命中次数加1 */
    route->hit_count.fetch_add(1);

    req.route_ = route;
    return true;
}

} // namespace server
} // namespace ic
