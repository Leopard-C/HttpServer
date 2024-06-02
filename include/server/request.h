/**
 * @file request.h
 * @brief HTTP请求.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_SERVER_REQUEST_H_
#define IC_SERVER_REQUEST_H_
#include <chrono>
#include <map>
#include <string>
#include <vector>
#include <jsoncpp/json/value.h>
#include "content_type.h"
#include "form_param.h"
#include "http_method.h"

namespace ic {
namespace server {

class ThreadInfo;
class HttpServer;
class RequestRaw;
class Session;
class Route;

/**
 * @brief HTTP请求.
 */
class Request {
public:
    friend class HttpServer;
    friend class Session;
    friend class Router;

public:
    Request(HttpServer* svr, RequestRaw* raw, const std::string& client_ip);
    Request(const Request&) = delete;
    Request& operator=(const Request&) = delete;
    ~Request();

    /**
     * @brief 获取原生boost.beast库中的HTTP请求对象.
     * 
     * @note 需要包含头文件 <server/request_raw.h>
     */
    const RequestRaw* raw() const { return raw_; }

    /**
     * @brief 获取服务器对象.
     */
    HttpServer* svr() const { return svr_; }

    /**
     * @brief 请求方法.
     */
    HttpMethod method() const { return method_; }

    /**
     * @brief 当前请求对应的ID.
     * 
     * @details 服务器运行期间唯一.
     */
    int64_t id() const { return id_; }

    /**
     * @brief 当前请求路径(/path/to/resource).
     */
    const std::string& path() const { return path_; }

    /**
     * @brief 当前请求命中的路由对象.
     */
    const Route* route() const { return route_; }

    /**
     * @brief 当前请求所在的线程信息.
     */
    const ThreadInfo* thread_info() const { return thread_info_; }

    /**
     * @brief 获取内容类型.
     */
    const ContentType& content_type() const { return content_type_; }

    /**
     * @brief 获取请求body.
     */
    const std::string& body() const;

    /**
     * @brief 获取自定义信息.
     */
    Json::Value& custom_data() { return custom_data_; }
    Json::Value& custom_data(const char* key) { return custom_data_[key]; }
    Json::Value& custom_data(const std::string& key) { return custom_data_[key]; }

    /**
     * @brief 请求到达的时间戳.
     */
    const std::chrono::system_clock::time_point& arrive_timepoint() const { return arrive_timepoint_; }

    /**
     * @brief 处理请求(执行回调函数)耗时.
     */
    const std::chrono::nanoseconds& time_consumed_handle() const { return time_consumed_handle_; }

    /**
     * @brief 请求(解析URL、解析body、预处理、处理等)总耗时.
     */
    const std::chrono::nanoseconds& time_consumed_total() const { return time_consumed_total_; }

    /**
     * @brief 是否带有指定的header.
     * 
     * @param name 字段名称，不区分大小写
     */
    bool HasHeader(const std::string& name) const;

    /**
     * @brief 获取请求头.
     * 
     * @param  name 字段名称，不区分大小写
     * @return std::string 字段值，如果不存在则返回空字符串
     */
    std::string GetHeader(const std::string& name) const;

    /**
     * @brief 获取请求头(可能存在多个同名的).
     * 
     * @param  name 字段名称，不区分大小写
     * @return std::vector<std::string> 字段值列表
     */
    std::vector<std::string> GetHeaders(const std::string& name) const;

    /**
     * @brief 获取User-Agent.
     */
    std::string GetUserAgent() const;

    /**
     * @brief 请求来源客户端IP地址.
     * 
     * @details 如果经过（正向/反向）代理，获取的是与当前服务器直接建立TCP连接的客户端IP.
     */
    const std::string& client_ip() const { return client_ip_; }

    /**
     * @brief 请求来源客户端的真实IP地址.
     * 
     * @details 依赖 X-Forwarded-For 请求头，存在伪造的可能性.
     */
    const std::string& client_real_ip() const { return client_real_ip_; }

    /**
     * @brief 获取本次请求携带的所有cookie.
     */
    const std::multimap<std::string, std::string>& cookies() const { return cookies_; }

    /**
     * @brief 获取本次请求的某项cookie.
     */
    const std::string& GetCookie(const std::string& name, bool* exist = nullptr) const;

    /**
     * @brief 正则路由的匹配项.
     * 
     * @details 例如对于正则路由 /article/post/(.+)/([0-9]+), 用户请求 /article/post/Tom/16
     * @details GetRouteRegexMatch(0) 返回第1个匹配项 Tom
     * @details GetRouteRegexMatch(1) 返回第2个匹配项 16
     * 
     * @param index 下标索引，第index个匹配项，从0开始编号
     */
    const std::string& GetRouteRegexMatch(size_t index) const { return route_regex_match_[index]; }

    /**
     * @brief 获取URL中的指定名称的参数值.
     */
    const std::string& GetUrlParam(const std::string& name, bool* exist = nullptr) const;

    /**
     * @brief 内容类型为application/x-www-form-urlencoded时，获取body中指定名称的参数.
     * 
     * @note content_type().IsApplicationXWwwFormUrlEncoded() == true
     */
    const std::string& GetBodyParam(const std::string& name, bool* exist = nullptr) const;

    /**
     * @brief 内容类型为multipart/form-data，获取解析后的指定名称的表单项.
     * 
     * @note content_type().IsMultipartFormData() == true
     * 
     * @return const FormParam* 如果不存在该参数，则返回nullptr
     */
    const FormParam* GetFormParam(const std::string& name) const;

    /**
     * @brief 内容类型为application/json时，获取解析后的json对象中指定字段名称的值.
     * 
     * @note content_type().IsApplicationJson() == true
     * 
     * @return const Json::Value& 通过 Json::Value::isNull() 判断参数是否存在
     */
    const Json::Value& GetJsonParam(const std::string& name) const;

    /**
     * @brief 内容类型为application/json时，获取解析后的json数组中指定下标的json对象.
     * 
     * @note content_type().IsApplicationJson() == true
     * 
     * @return const Json::Value& 通过 Json::Value::isNull() 判断参数是否存在
     */
    const Json::Value& GetJsonParam(size_t index) const;

    /**
     * @brief 获取所有的URL参数名和参数值.
     */
    const std::multimap<std::string, std::string>& url_params() const { return url_params_; }

    /**
     * @brief 内容类型为application/x-www-form-urlencoded时，获取body中所有的参数.
     */
    const std::multimap<std::string, std::string>& body_params() const { return body_params_; }

    /**
     * @brief 内容类型为multipart/form-data时，获取form表单所有内容.
     */
    const std::multimap<std::string, const FormParam*>& form_params() const { return form_params_; };

    /**
     * @brief 内容类型为application/json时，获取解析后的json对象.
     */
    const Json::Value& json_params() const { return json_params_; }

private:
    void LogAccessVerbose();

    void ParseBasic();
    void ParseClientRealIp();
    void ParseCookie();
    bool ParseBody();

    bool ParseBody_XWwwFormUrlEncoded(const std::string& body);
    bool ParseBody_MultipartFormData(const std::string& body);
    bool ParseBody_ApplicationJson(const std::string& body);
    void ParseUrlParams(const char* str, size_t len);

private:
    RequestRaw* raw_{nullptr};
    HttpServer* svr_{nullptr};

    /**
     * @brief 客户端地址.
     * 
     * @details 如果经过(反向)代理，可能获取的是(反向)代理服务器的地址.
     */
    std::string client_ip_;

    /** 客户端真实地址 */
    std::string client_real_ip_;

    /** 请求到达时间戳 */
    std::chrono::system_clock::time_point arrive_timepoint_;

    /** 处理请求耗时 */
    std::chrono::nanoseconds time_consumed_handle_{0};

    /** 解析、处理请求总耗时 */
    std::chrono::nanoseconds time_consumed_total_{0};

    /** 请求方法 */
    HttpMethod method_{HttpMethod::kNotSupport};

    /** 本次请求的ID(全局唯一) */
    int64_t id_{-1};

    /** 请求路径，如/user/getInfo */
    std::string path_;

    /** 当前请求命中的路由 */
    Route* route_{nullptr};

    /** 当前请求所在的线程信息 */
    ThreadInfo* thread_info_{nullptr};

    /** 内容类型 */
    ContentType content_type_;

    /**
     * @brief 正则路由匹配项.
     */
    std::vector<std::string> route_regex_match_;

    /** 自定义信息，程序内修改和使用 */
    Json::Value custom_data_;

    /** cookie */
    std::multimap<std::string, std::string> cookies_;

    /** URL中的参数 */
    std::multimap<std::string, std::string> url_params_;

    /** (1)内容类型为application/x-www-form-urlencoded时body中的参数 */
    std::multimap<std::string, std::string> body_params_;

    /** (2)内容类型为multipart/form-data，存放解析后的表单对象 */
    std::multimap<std::string, const FormParam*> form_params_;

    /** (3)内容类型为application/json时，存放解析后的json对象 */
    Json::Value json_params_;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_REQUEST_H_
