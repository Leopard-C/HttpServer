/**
 * @file config.h
 * @brief HTTP服务器配置.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 *
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_SERVER_CONFIG_H_
#define IC_SERVER_CONFIG_H_
#include <string>
#include <jsoncpp/json/value.h>

namespace ic {
namespace server {

/**
 * @brief HTTP服务器配置.
 */
class HttpServerConfig {
public:
    /**
     * @brief 从json文件读取配置.
     *
     * @details 配置文件示例
     * @details {
     * @details   "ip": "0.0.0.0",
     * @details   "port": 8099,
     * @details   "num_threads": 2,
     * @details   "xxh64_seed": 268435455,
     * @details   "version": "1.0.0",
     * @details   "tcp_stream_timeout_ms": 15000,
     * @details   "body_limit": 11534334,
     * @details   "log_access_verbose": false
     * @details }
     */
    bool ReadFromFile(const std::string& filename);

    /**
     * @brief 将配置写入json文件.
     */
    bool WriteToFile() const;

    /**
     * @brief 写入json对象中.
     */
    Json::Value ToJson() const;

public:
    unsigned int num_threads() const { return num_threads_; }
    unsigned int port() const { return port_; }
    const std::string& ip() const { return ip_; }
    bool log_access_verbose() const { return log_access_verbose_; }
    unsigned int tcp_stream_timeout_ms() const { return tcp_stream_timeout_ms_; }
    uint64_t body_limit() const { return body_limit_; }
    uint64_t xxh64_seed() const { return xxh64_seed_; }
    const std::string& version() const { return version_; }

    void set_num_threads(unsigned int num_threads) { num_threads_ = num_threads; }
    void set_port(unsigned int port) { port_ = port; }
    void set_ip(const std::string& ip) { ip_ = ip; }
    void set_log_access_verbose(bool verbose) { log_access_verbose_ = verbose; }
    void set_tcp_stream_timeout_ms(unsigned int timeout_ms) { tcp_stream_timeout_ms_ = timeout_ms; }
    void set_body_limit(uint64_t body_limit) { body_limit_ = body_limit; }
    void set_xxh64_seed(uint64_t seed) { xxh64_seed_ = seed; }
    void set_version(const std::string& version) { version_ = version; }

private:
    /** 线程数量 */
    unsigned int num_threads_{2};

    /** 监听端口号 */
    unsigned int port_{8099};

    /** IP地址 */
    std::string ip_{"0.0.0.0"};

    /**
     * @brief 日志记录时，是否记录详细信息.
     *
     * @details 建议调试时才设为true，正式环境设为false
     */
    bool log_access_verbose_{false};

    /** tcp超时时间(单位:毫秒)，0表示不限制，默认15s */
    unsigned int tcp_stream_timeout_ms_{15000};

    /** body大小限制(单位:字节)，默认11MB */
    uint64_t body_limit_{1024 * 1024 * 11};

    /** XXH64摘要算法种子值 */
    uint64_t xxh64_seed_{0xfffffff};

    /** HTTP Server版本号 */
    std::string version_{"1.0.0"};

private:
    /** 配置文件路径 */
    std::string filename_;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_CONFIG_H_
