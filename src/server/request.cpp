#include "request.h"
#include "request_raw.h"
#include "http_server.h"
#include "multipart_parser.h"
#include "util/string/isprint.h"
#include "util/url_code.h"
#include <boost/asio/dispatch.hpp>
#include <jsoncpp/json/json.h>
#include <log/logger.h>

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>

namespace ic {
namespace server {

const static std::string s_empty_string;

static inline std::string to_string(const boost::string_view& sv) {
    return { sv.data(), sv.size() };
}

static HttpMethod from_beast_http_method(boost::beast::http::verb method) {
    using verb = boost::beast::http::verb;
    switch (method) {
        case verb::get:  return HttpMethod::kGET;
        case verb::head: return HttpMethod::kHEAD;
        case verb::post: return HttpMethod::kPOST;
        case verb::put:  return HttpMethod::kPUT;
        case verb::delete_: return HttpMethod::kDELETE;
        case verb::connect: return HttpMethod::kCONNECT;
        case verb::options: return HttpMethod::kOPTIONS;
        case verb::trace: return HttpMethod::kTRACE;
        case verb::patch: return HttpMethod::kPATCH;
        default: return HttpMethod::kNotSupport;
    }
}

/**
 * @brief 分隔KeyValue格式的字符串
 */
static void split_key_value(const char* str, size_t len,
    const char* outer_delimiter, size_t outer_delimiter_len,
    const char* inner_delimiter, size_t inner_delimiter_len,
    bool trim_key, bool trim_value,
    std::multimap<std::string, std::string>* result)
{
    result->clear();
    StringView sv(str, len);
    std::vector<StringView> key_values = sv.Split(outer_delimiter, outer_delimiter_len, true);
    StringView key, value;
    std::string key_decoded, value_decoded;
    for (const auto& key_value : key_values) {
        auto pos = key_value.Find(inner_delimiter, inner_delimiter_len);
        if (pos == std::string::npos) {
            pos = key_value.length();
        }
        key = key_value.SubStr(0, pos);
        value = key_value.SubStr(pos + inner_delimiter_len);
        if (trim_key) {
            key.Trim();
        }
        if (trim_value) {
            value.Trim();
        }
        if (util::url_decode(key.data(), key.length(), &key_decoded) &&
            util::url_decode(value.data(), value.length(), &value_decoded))
        {
            result->emplace(key_decoded, value_decoded);
        }
    }
}

Request::Request(HttpServer* svr, RequestRaw* raw, const std::string& client_ip)
    : svr_(svr), raw_(raw), arrive_timepoint_(std::chrono::system_clock::now())
{
    id_ = svr_->current_request_id();
    client_ip_ = client_ip;
    client_real_ip_ = client_ip;
    ParseBasic();
}

Request::~Request() {
    for (auto iter = form_items_.begin(); iter != form_items_.end(); ++iter) {
        delete iter->second;
        iter->second = nullptr;
    }
}

/**
 * @brief 获取请求body.
 */
const std::string& Request::body() const {
    return raw_->body();
}

bool Request::HasHeader(const std::string& name) const {
    return raw_->find(name) != raw_->end();
}

std::string Request::GetHeader(const std::string& name) const {
    return to_string(raw_->operator[](name));
}

std::vector<std::string> Request::GetHeaders(const std::string& name) const {
    std::vector<std::string> headers;
    auto range = raw_->equal_range(name);
    for (auto iter = range.first; iter != range.second; ++iter) {
        headers.push_back(to_string(iter->value()));
    }
    return headers;
}

std::string Request::GetUserAgent() const {
    return to_string(raw_->operator[](http::field::user_agent));
}

/**
 * @brief 获取本次请求携带的所有cookie.
 */
const std::multimap<std::string, std::string>& Request::cookies() {
    if (!cookie_parsed_) {
        ParseCookie();
        cookie_parsed_ = true;
    }
    return cookies_;
}

const std::string& Request::GetCookie(const std::string& name, bool* exist/* = nullptr*/) {
    if (!cookie_parsed_) {
        ParseCookie();
        cookie_parsed_ = true;
    }
    auto it = cookies_.find(name);
    bool _exist = (it != cookies_.end());
    exist && (*exist = _exist);
    return _exist ? it->second : s_empty_string;
}

const std::string& Request::GetUrlParam(const std::string& name, bool* exist/* = nullptr*/) const {
    auto it = url_params_.find(name);
    bool _exist = (it != url_params_.end());
    exist && (*exist = _exist);
    return _exist ? it->second : s_empty_string;
}

const std::string& Request::GetBodyParam(const std::string& name, bool* exist/* = nullptr*/) const {
    auto it = body_params_.find(name);
    bool _exist = (it != url_params_.end());
    exist && (*exist = _exist);
    return _exist ? it->second : s_empty_string;
}

const Json::Value& Request::GetJsonParam(const std::string& name) const {
    if (json_params_.isObject()) {
        return json_params_[name];
    }
    return Json::Value::nullRef;
}

const Json::Value& Request::GetJsonParam(size_t index) const {
    if (json_params_.isArray() && (unsigned int)index < json_params_.size()) {
        return json_params_[(unsigned int)index];
    }
    return Json::Value::nullRef;
}

const FormItem* Request::GetFormItem(const std::string& name) const {
    auto it = form_items_.find(name);
    return (it != form_items_.end()) ? it->second : nullptr;
}

/**
 * @brief 日志记录.
 */
void Request::LogAccess() {
    LInfo("ACCESS {} {:.{}} IP:{}", to_string(method_), raw_->target().data(), raw_->target().length(), client_real_ip_);
    if (svr_->config().log_access_verbose()) {
        LogAccessVerbose();
    }
}

static std::string to_string(const std::multimap<std::string, std::string>& map) {
    if (map.empty()) {
        return "    <NONE>\n";
    }
    std::string msg;
    for (const auto& pair : map) {
        msg += "    " + pair.first + ": " + pair.second + '\n';
    }
    return msg;
}

/**
 * @brief 详细记录请求信息.
 */
void Request::LogAccessVerbose() {
    std::string msg;
    msg.reserve(512);
    msg += fmt::format(
        "\n------------------------------------------------\n"
        "  method: {}\n"
        "  path: {}\n"
        "  path id: {}\n"
        "  client ip: {}\n"
        "  client real ip: {}\n",
        to_string(method_), path_, path_id_, client_ip_, client_real_ip_
    );
    msg += "  headers:\n";
    for (auto iter = raw_->begin(); iter != raw_->end(); ++iter) {
        msg += "    " + to_string(iter->name_string()) + ": " + to_string(iter->value()) + '\n';
    }
    msg += fmt::format(
        "  cookies:\n{}"
        "  url params:\n{}"
        "  body params:\n{}",
        to_string(cookies()), to_string(url_params()), to_string(body_params())
    );
    msg += "  form items:\n";
    if (form_items_.empty()) {
        msg += "    <NONE>\n";
    }
    for (const auto& pair : form_items_) {
        const FormItem* item = pair.second;
        size_t max_len = std::min(item->content().length(), (size_t)512);
        auto content = item->content().SubStr(0, max_len);
        msg += fmt::format("    {}: [file={}] [content_type={}] [charset={}] [{}] {}\n",
            pair.first, item->is_file(), item->content_type(), item->charset(), item->content().length(),
            util::isprint(content.data(), content.length()) ? content.ToString() : "<NOT PRINTABLE>"
        );
    }
    msg += "------------------------------------------------";
    LInfo(msg);
}

/**
 * @brief 解析请求中的基本内容.
 * 
 * @details 客户端IP、请求路径、URL参数.
 */
void Request::ParseBasic() {
    // 1. client real ip
    ParseClientRealIp();

    // 2. http method
    method_ = from_beast_http_method(raw_->method());

    // 3. request path & url params
    auto tgt = raw_->target();
    size_t pos = tgt.find('?');
    if (pos == boost::string_view::npos) {
        path_ = to_string(tgt);
    }
    else {
        path_ = to_string(tgt.substr(0, pos));
        auto url_params = tgt.substr(pos + 1);
        ParseUrlParams(url_params.data(), url_params.size());
    }
    path_id_ = svr_->XXH64(path_);

    // 4. content type
    auto content_type = raw_->operator[](http::field::content_type);
    content_type_.Parse(StringView(content_type.data(), content_type.size()));
}

/**
 * @brief 解析客户端真实IP.
 * 
 * @details 有代理的情况，经过服务器（如Nginx、squid）代理.
 */
void Request::ParseClientRealIp() {
    auto forwarded = raw_->operator[]("X-Forwarded-For");
    if (forwarded.empty()) {
        return;
    }
    boost::system::error_code ec;
    net::ip::address forwarded_ip;
    auto pos = forwarded.find(',');
    if (pos == boost::string_view::npos) {
        forwarded_ip = net::ip::address::from_string(to_string(forwarded), ec);
    }
    else {
        forwarded_ip = net::ip::address::from_string(to_string(forwarded.substr(0, pos)), ec);
    }
    if (ec) {
        LWarn("Invalid forwarded ip address: {}", to_string(forwarded));
        return;
    }
    client_real_ip_ = forwarded_ip.to_string();
}

/**
 * @brief 解析URL中的参数.
 */
void Request::ParseUrlParams(const char* str, size_t len) {
    split_key_value(str, len, "&", 1, "=", 1, false, false, &url_params_);
}

/**
 * @brief 解析cookie.
 */
void Request::ParseCookie() {
    auto cookies = raw_->operator[](http::field::cookie);
    split_key_value(cookies.data(), cookies.length(), ";", 1, "=", 1, true, true, &cookies_);
}

#pragma region PARSE_BODY
/**********************************************************************************
 *
 *   解析不同内容类型的body，默认只解析以下3种(Content-Type)
 *      1. application/x-www-form-urlencoded  ==> body_params_
 *      2. application/json                   ==> json_params_
 *      3. multipart/form-data                ==> form_items_
 *
**********************************************************************************/
bool Request::ParseBody() {
    const std::string& body = raw_->body();
    if (body.empty() || (method_ != HttpMethod::kPOST && method_ != HttpMethod::kPUT && method_ != HttpMethod::kPATCH && method_ != HttpMethod::kDELETE)) {
        return true;
    }
    if (content_type_.IsApplicationXWwwFormUrlEncoded()) {
        return ParseBody_XWwwFormUrlEncoded(body);
    }
    else if (content_type_.IsApplicationJson()) {
        return ParseBody_ApplicationJson(body);
    }
    else if (content_type_.IsMultipartFormData()) {
        return ParseBody_MultipartFormData(body);
    }
    else {
        LDebug("Unhandled Content-Type: {}", content_type_.type());
        return true;
    }
}

/**
 * @brief 解析application/x-www-form-urlencoded
 */
bool Request::ParseBody_XWwwFormUrlEncoded(const std::string& body) {
    split_key_value(body.data(), body.length(), "&", 1, ";", 1, false, false, &body_params_);
    return true;
}

/**
 * @brief 解析application/json
 */
bool Request::ParseBody_ApplicationJson(const std::string& body) {
    Json::Reader reader;
    if (!reader.parse(body, json_params_, false)) {
        LError("Invalid json body:\n{} ", reader.getFormattedErrorMessages());
        return false;
    }
    if (!json_params_.isObject() && !json_params_.isArray()) {
        LError("Invalid json body: the top level of json must be an object or array");
        return false;
    }
    return true;
}

/**
 * @brief 解析multipart/form-data
 */
bool Request::ParseBody_MultipartFormData(const std::string& body) {
    if (content_type_.boundary().empty()) {
        LWarn("Missing boundary. Content-Type=multipart/form-data");
        return true;
    }
    MultipartParser parser(body);
    return parser.Parse("--" + content_type_.boundary(), &form_items_);
}
#pragma endregion PARSE_BODY

} // namespace server
} // namespace ic
