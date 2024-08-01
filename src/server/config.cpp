#include "server/config.h"
#include <fstream>
#include <jsoncpp/json/json.h>

#define ERROR_KEY(key) \
    fprintf(stderr, "Invalid key @[%s] in configuration file '%s'", key, filename.c_str());\
    return false

#define CHECK_STRING(node, key, value) \
    if (!node[key].isNull()){\
        if (!node[key].isString()) {\
            ERROR_KEY(key);\
        }\
        std::string value##Tmp = node[key].asString();\
        if (value##Tmp.empty()) {\
            ERROR_KEY(key);\
        }\
        value = value##Tmp;\
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
        fprintf(stderr, "Open json file '%s' failed", filename.c_str());
        return false;
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(ifs, root, false)) {
        fprintf(stderr, "Parse json file '%s' failed", filename.c_str());
        return false;
    }

    CHECK_UINT(root, "num_threads", num_threads_);
    CHECK_UINT(root, "port", port_);
    CHECK_STRING(root, "ip", ip_);
    CHECK_BOOL(root, "reuse_address", reuse_address_);
    CHECK_BOOL(root, "log_access", log_access_);
    CHECK_BOOL(root, "log_access_verbose", log_access_verbose_);
    CHECK_UINT(root, "tcp_stream_timeout_ms", tcp_stream_timeout_ms_);
    CHECK_UINT64(root, "body_limit", body_limit_);
    CHECK_STRING(root, "version", version_);

    if (port_ > 65535) {
        ERROR_KEY("port");
    }
    if (num_threads_ < 1) {
        ERROR_KEY("num_threads");
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
            fprintf(stderr, "Filename is empty");
            return false;
        }
        filename = filename_;
    }
    std::string content = ToJson().toStyledString();
    std::ofstream ofs(filename);
    if (!ofs) {
        fprintf(stderr, "Open file failed. '%s'", filename.c_str());
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
    root["num_threads"] = num_threads_;
    root["ip"] = ip_;
    root["port"] = port_;
    root["reuse_address"] = reuse_address_;
    root["log_access"] = log_access_;
    root["log_access_verbose"] = log_access_verbose_;
    root["tcp_stream_timeout_ms"] = tcp_stream_timeout_ms_;
    root["body_limit"] = body_limit_;
    root["version"] = version_;
    return root;
}

} // namespace server
} // namespace ic
