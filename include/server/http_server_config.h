/**
 * @file http_server_config.h
 * @brief HTTP服务器配置.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 *
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_SERVER_HTTP_SERVER_CONFIG_H_
#define IC_SERVER_HTTP_SERVER_CONFIG_H_
#include <string>
#include <jsoncpp/json/value.h>

namespace ic {
namespace server {

/**
 * @brief HTTP服务器配置.
 */
class HttpServerConfig {
public:
    struct Endpoint {
        Endpoint() : Endpoint("0.0.0.0", 8099, true) {}
        Endpoint(const std::string& ip, unsigned short port, bool reuse_address) : ip(ip), port(port), reuse_address(reuse_address) {}
        std::string ip;
        unsigned short port;
        bool reuse_address;
    };

    /**
     * @brief 从json文件读取配置.
     *
     * @details 配置文件示例
     * @details {
     * @details   "min_num_threads": 2,
     * @details   "max_num_threads": 8,
     * @details   "version": "1.0.0",
     * @details   "tcp_stream_timeout_ms": 15000,
     * @details   "body_limit": 11534334,
     * @details   "log_access": true
     * @details   "log_access_verbose": false,
     * @details   "endpoints": [
     * @details     {
     * @details       "ip": "0.0.0.0",
     * @details       "port": 8099,
     * @details       "reuse_address": true
     * @details     }
     * @details   ] 
     * @details }
     */
    bool ReadFromFile(const std::string& filename);

    /**
     * @brief 将配置写入json文件.
     * 
     * @param filename 文件名称，如果为空，将使用调用`ReadFromFile`时传递的`filename`.
     */
    bool WriteToFile(std::string filename = "") const;

    /**
     * @brief 写入json对象中.
     */
    Json::Value ToJson() const;

public:
    unsigned int min_num_threads() const { return min_num_threads_; }
    unsigned int max_num_threads() const { return max_num_threads_; }
    const std::vector<Endpoint>& endpoints() const { return endpoints_; }
    std::vector<Endpoint>& endpoints() { return endpoints_; }
    bool log_access() const { return log_access_; }
    bool log_access_verbose() const { return log_access_verbose_; }
    unsigned int tcp_stream_timeout_ms() const { return tcp_stream_timeout_ms_; }
    uint64_t body_limit() const { return body_limit_; }
    const std::string& version() const { return version_; }

    void set_min_num_threads(unsigned int min_num_threads) { min_num_threads_ = min_num_threads; }
    void set_max_num_threads(unsigned int max_num_threads) { max_num_threads_ = max_num_threads; }
    void add_endpoint(const Endpoint& endpoint) { endpoints_.push_back(endpoint); }
    void add_endpoint(const std::string& ip, unsigned short port, bool reuse_address = true) { endpoints_.emplace_back(ip, port, reuse_address); }
    void set_log_access(bool log_access) { log_access_ = log_access; }
    void set_log_access_verbose(bool verbose) { log_access_verbose_ = verbose; }
    void set_tcp_stream_timeout_ms(unsigned int timeout_ms) { tcp_stream_timeout_ms_ = timeout_ms; }
    void set_body_limit(uint64_t body_limit) { body_limit_ = body_limit; }
    void set_version(const std::string& version) { version_ = version; }

private:
    /** 线程数量最小值 */
    unsigned int min_num_threads_{2};

    /** 线程数量最大值 */
    unsigned int max_num_threads_{8};

    /** 监听地址 */
    std::vector<Endpoint> endpoints_;

    /**
     * @brief 是否打印用户请求.
     */
    bool log_access_{true};

    /**
     * @brief 是否记录详细的用户请求信息.
     *
     * @details 建议调试时才设为true，正式环境设为false
     */
    bool log_access_verbose_{false};

    /** tcp超时时间(单位:毫秒)，0表示不限制，默认15s */
    unsigned int tcp_stream_timeout_ms_{15000};

    /** body大小限制(单位:字节)，默认11MB */
    uint64_t body_limit_{1024 * 1024 * 11};

    /** HTTP Server版本号 */
    std::string version_{"1.0.0"};

private:
    /** 配置文件路径 */
    std::string filename_;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_HTTP_SERVER_CONFIG_H_
