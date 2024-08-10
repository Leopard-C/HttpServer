/**
 * @file response.h
 * @brief HTTP响应.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_SERVER_RESPONSE_H_
#define IC_SERVER_RESPONSE_H_
#include <map>
#include <jsoncpp/json/value.h>

namespace ic {
namespace server {

class Session;
class HttpServer;
class HttpCookie;

/**
 * @brief HTTP响应.
 */
class Response {
public:
    friend class Session;

    Response(HttpServer* svr);
    Response(Response&&) = delete;
    Response(const Response&) = delete;
    Response& operator=(const Response&) = delete;
    ~Response();

    void set_keep_alive(bool keep_alive) { keep_alive_ = keep_alive; }
    bool keep_alive() const { return keep_alive_; }

    bool is_file_body() const { return is_file_body_; }

    /**
     * @brief 设置响应头.
     */
    void SetHeader(const std::string& name, const std::string& value);

    /**
     * @brief 移除指定名称的响应头.
     */
    void RemoveHeader(const std::string& name);

    /**
     * @brief 获取所有的响应头.
     */
    std::multimap<std::string, std::string>& headers() { return headers_; }

    /**
     * @brief 添加Set-Cookie响应头.
     */
    void SetCookie(const HttpCookie& cookie);

    /**
     * @brief 移除所有的Set-Cookie响应头.
     */
    void RemoveAllCookies();

    /**
     * @brief 响应文本内容.
     */
    void SetStringBody(unsigned int status_code);
    void SetStringBody(const std::string& body, const std::string& content_type);
    void SetStringBody(unsigned int status_code, const std::string& body, const std::string& content_type);

    /**
     * @brief 响应JSON格式的文本内容.
     */
    void SetJsonBody(const Json::Value& root);
    void SetJsonBody(unsigned int status_code, const Json::Value& root);

    /**
     * @brief 响应文件内容(文件路径UTF8编码).
     */
    void SetFileBody(const std::string& filepath, const std::string& content_type = "");
    void SetFileBody(unsigned int status_code, const std::string& filepath, const std::string& content_type = "");

    void SetBadRequest(const std::string& why = "Bad Request!");

private:
    void SetContentType(const std::string& content_type);

private:
    HttpServer* svr_;
    bool keep_alive_{true};
    bool is_file_body_{false};
    unsigned int status_code_{200U};
    std::string string_body_;
    std::string filepath_;  // Response file body. The path must be utf-8 encoded.
    std::multimap<std::string, std::string> headers_;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_RESPONSE_H_
