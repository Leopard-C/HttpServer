#include "server/router.h"
#include "server/http_server.h"
#include "server/logger.h"
#include "server/request.h"
#include "server/response.h"
#include "server/status/base.h"

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

Json::Value Route::ToJson() const {
    Json::Value root;
    root["methods"] = GetMethodsString();
    root["hit_count"] = (uint64_t)hit_count;
    root["path"] = path;
    root["description"] = description;
    for (const auto& config : configuration) {
        root["configuration"][config.first] = config.second;
    }
    return root;
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
        root["code"] = status::kInvalidPath;
        root["msg"] = "Invalid request path";
        res.SetJsonBody(root);
    };
    cb_invalid_method_ = [](Request& req, Response& res) {
        Json::Value root;
        root["code"] = status::kInvalidMethod;
        root["msg"] = "Invalid http method";
        res.SetJsonBody(root);
    };
}

Router::~Router() {
}

/**
 * @brief 添加静态路由，如果已存在则覆盖.
 */
bool Router::AddStaticRoute(StaticRoutePtr route) {
    WriteLock lck(mutex_);
    if (!CheckRoute(route)) {
        return false;
    }
    DeleteRoute_WithoutLock(route->path);
    svr_->logger()->Debug(LOG_CTX, "Add static route: %4s %s", route->GetMethodsString().c_str(), route->path.c_str());
    routes_.emplace(route->path, route);
    static_routes_.emplace(route->path, route);
    return true;
}

bool Router::AddStaticRoute(const std::string& path, int methods,
    std::function<void(Request&, Response&)> callback,
    const std::string& description/* = ""*/,
    const std::unordered_map<std::string, std::string>& configuration/* = {}*/)
{
    auto route = std::make_shared<StaticRoute>(path, methods, callback, description, configuration);
    return AddStaticRoute(route);
}

bool Router::AddStaticRoute(const std::string& path, int methods,
    std::function<void(Request&, Json::Value&)> callback,
    const std::string& description/* = ""*/,
    const std::unordered_map<std::string, std::string>& configuration/* = {}*/)
{
    auto route = std::make_shared<StaticRoute>(path, methods, callback, description, configuration);
    return AddStaticRoute(route);
}

/**
 * @brief 添加正则路由，如果已存在则覆盖.
 */
bool Router::AddRegexRoute(RegexRoutePtr route) {
    WriteLock lck(mutex_);
    if (!CheckRoute(route)) {
        return false;
    }
    DeleteRoute_WithoutLock(route->path);
    svr_->logger()->Debug(LOG_CTX, "Add  regex route: %4s %s", route->GetMethodsString().c_str(), route->path.c_str());
    routes_.emplace(route->path, route);
    regex_routes_.emplace_back(route);
    return true;
}

bool Router::AddRegexRoute(const std::string& path, int methods,
    std::function<void(Request&, Response&)> callback,
    const std::string& description/* = ""*/,
    const std::unordered_map<std::string, std::string>& configuration/* = {}*/)
{
    try {
        auto route = std::make_shared<RegexRoute>(path, methods, callback, description, configuration);
        return AddRegexRoute(route);
    }
    catch (const REGEX_NAMESPACE::regex_error& ex) {
        svr_->logger()->Error(LOG_CTX, "Invalid regex pattern: %s, can not add to router. %s", path.c_str(), ex.what());
        return false;
    }
}

bool Router::AddRegexRoute(const std::string& path, int methods,
    std::function<void(Request&, Json::Value&)> callback,
    const std::string& description/* = ""*/,
    const std::unordered_map<std::string, std::string>& configuration/* = {}*/)
{
    try {
        auto route = std::make_shared<RegexRoute>(path, methods, callback, description, configuration);
        return AddRegexRoute(route);
    }
    catch (const REGEX_NAMESPACE::regex_error& ex) {
        svr_->logger()->Error(LOG_CTX, "Invalid regex pattern: %s, can not add to router. %s", path.c_str(), ex.what());
        return false;
    }
}


/**
 * @brief 检查路由是否合法.
 */
bool Router::CheckRoute(RoutePtr route) const {
    if (route->path.empty()) {
        svr_->logger()->Error(LOG_CTX, "Empty path, can not add to router");
        return false;
    }
    if (!(route->methods & (int)HttpMethod::kALL)) {
        svr_->logger()->Error(LOG_CTX, "Invalid http methods, can not add to router");
        return false;
    }
    return true;
}

/**
 * @brief 删除路由.
 */
void Router::DeleteRoute(const std::string& path) {
    WriteLock lck(mutex_);
    DeleteRoute_WithoutLock(path);
}

/**
 * @brief 删除路由(无锁).
 */
void Router::DeleteRoute_WithoutLock(const std::string& path) {
    if (routes_.find(path) == routes_.end()) {
        return;
    }
    auto iter = static_routes_.find(path);
    if (iter != static_routes_.end()) {
        /* 删除静态路由 */
        static_routes_.erase(iter);
    }
    else {
        /* 删除正则路由 */
        for (auto iter = regex_routes_.begin(); iter != regex_routes_.end(); ++iter) {
            if ((*iter)->path == path) {
                regex_routes_.erase(iter);
                break;
            }
        }
    }
    svr_->logger()->Debug(LOG_CTX, "Route deleted. path: %s", path.c_str());
}

/**
 * @brief 检查请求是否命中已注册的路由.
 */
bool Router::HitRoute(Request& req, Response& res) {
    ReadLock lck(mutex_);
    RoutePtr route;
    auto iter = static_routes_.find(req.path());
    if (iter != static_routes_.end()) {
        route = iter->second;
    }
    else {
        try {
            REGEX_NAMESPACE::smatch match;
            for (auto& r : regex_routes_) {
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
            svr_->logger()->Error(LOG_CTX, "regex_match error, path: %s, err_msg: %s", req.path().c_str(), ex.what());
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

/**
 * @brief 获取路由.
 */
RouteConstPtr Router::GetRoute(const std::string& path) {
    ReadLock lck(mutex_);
    auto find_iter = routes_.find(path);
    if (find_iter != routes_.end()) {
        return find_iter->second;
    }
    return nullptr;
}

std::unordered_map<std::string, RouteConstPtr> Router::routes() {
    ReadLock lck(mutex_);
    return std::unordered_map<std::string, RouteConstPtr>(routes_.begin(), routes_.end());
}

std::unordered_map<std::string, StaticRouteConstPtr> Router::static_routes() {
    ReadLock lck(mutex_);
    return std::unordered_map<std::string, StaticRouteConstPtr>(static_routes_.begin(), static_routes_.end());
}

std::vector<RegexRouteConstPtr> Router::regex_routes() {
    ReadLock lck(mutex_);
    return std::vector<RegexRouteConstPtr>(regex_routes_.begin(), regex_routes_.end());
}

void Router::set_cb_invalid_path(Route::ResponseCallback cb) {
    WriteLock lck(mutex_);
    if (cb) {
        cb_invalid_path_ = cb;
    }
}

void Router::set_cb_invalid_method(Route::ResponseCallback cb) {
    WriteLock lck(mutex_);
    if (cb) {
        cb_invalid_method_ = cb;
    }
}

} // namespace server
} // namespace ic
