#include "server/http_server_config.h"
#include <fstream>
#include <jsoncpp/json/json.h>

#define ERROR_KEY(key) \
    fprintf(stderr, "Invalid key @[%s] in configuration file '%s'\n", key, filename.c_str());\
    return false

#define CHECK_STRING(node, key, value) \
    if (!node[key].isNull()){\
        if (!node[key].isString()) {\
            ERROR_KEY(key);\
        }\
        std::string value_tmp = node[key].asString();\
        if (value_tmp.empty()) {\
            ERROR_KEY(key);\
        }\
        value = value_tmp;\
    }

#define CHECK_UINT64(node, key, value) \
    if (!node[key].isNull()){\
        if (!node[key].isUInt64()) {\
            ERROR_KEY(key);\
        }\
        value = node[key].asUInt64();\
    }

#define CHECK_UINT(node, key, value) \
    if (!node[key].isNull()){\
        if (!node[key].isUInt()) {\
            ERROR_KEY(key);\
        }\
        value = node[key].asUInt();\
    }

#define CHECK_BOOL(node, key, value) \
    if (!node[key].isNull()){\
        if (!node[key].isBool()) {\
            ERROR_KEY(key);\
        }\
        value = node[key].asBool();\
    }

namespace ic {
namespace server {

bool HttpServerConfig::ReadFromFile(const std::string& filename) {
    filename_ = filename;
    std::ifstream ifs(filename);
    if (!ifs) {
        fprintf(stderr, "Open json file '%s' failed\n", filename.c_str());
        return false;
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(ifs, root, false) || !root.isObject()) {
        fprintf(stderr, "Parse json file '%s' failed\n", filename.c_str());
        return false;
    }

    CHECK_UINT(root, "min_num_threads", min_num_threads_);
    CHECK_UINT(root, "max_num_threads", max_num_threads_);
    CHECK_BOOL(root, "log_access", log_access_);
    CHECK_BOOL(root, "log_access_verbose", log_access_verbose_);
    CHECK_UINT(root, "tcp_stream_timeout_ms", tcp_stream_timeout_ms_);
    CHECK_UINT64(root, "body_limit", body_limit_);
    CHECK_STRING(root, "version", version_);

    auto& v_endpoints = root["endpoints"];
    if (!v_endpoints.isArray()) {
        ERROR_KEY("endpoints");
        return false;
    }

    unsigned int num_endpoints = v_endpoints.size();
    endpoints_.resize(num_endpoints);
    for (unsigned int i = 0; i < num_endpoints; ++i) {
        if (!v_endpoints[i].isObject()) {
            ERROR_KEY("endpoints");
            return false;
        }
        CHECK_BOOL(v_endpoints[i], "reuse_address", endpoints_[i].reuse_address);
        CHECK_STRING(v_endpoints[i], "ip", endpoints_[i].ip);
        unsigned int port = 0;
        CHECK_UINT(v_endpoints[i], "port", port);
        if (port > 65535) {
            ERROR_KEY("port");
        }
        endpoints_[i].port = (unsigned short)port;
    }

    return true;
}

/**
 * @brief 将配置写入json文件.
 * 
 * @param filename 文件名称，如果为空，将使用调用`ReadFromFile`时传递的`filename`.
 */
bool HttpServerConfig::WriteToFile(std::string filename/* = ""*/) const {
    if (filename.empty()) {
        if (filename_.empty()) {
            fprintf(stderr, "Filename is empty\n");
            return false;
        }
        filename = filename_;
    }
    std::string content = ToJson().toStyledString();
    std::ofstream ofs(filename);
    if (!ofs) {
        fprintf(stderr, "Open file failed. '%s'\n", filename.c_str());
        return false;
    }
    ofs << content;
    return true;
}

/**
 * @brief 写入json对象中.
 */
Json::Value HttpServerConfig::ToJson() const {
    Json::Value root;
    root["min_num_threads"] = min_num_threads_;
    root["max_num_threads"] = max_num_threads_;
    root["log_access"] = log_access_;
    root["log_access_verbose"] = log_access_verbose_;
    root["tcp_stream_timeout_ms"] = tcp_stream_timeout_ms_;
    root["body_limit"] = body_limit_;
    root["version"] = version_;
    for (const auto& endpoint : endpoints_) {
        Json::Value v_endpoint;
        v_endpoint["ip"] = endpoint.ip;
        v_endpoint["port"] = (unsigned int)endpoint.port;
        v_endpoint["reuse_address"] = endpoint.reuse_address;
        root["endpoints"].append(v_endpoint);
    }
    return root;
}

} // namespace server
} // namespace ic
