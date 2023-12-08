#ifndef IC_SERVER_UTIL_MIME_H_
#define IC_SERVER_UTIL_MIME_H_
#include <string>

namespace ic {
namespace server {
namespace util {

/**
 * @brief 获取文件扩展名对应的MIME类型.
 */
const char* get_mimetype(const char* ext);

/**
 * @brief 获取文件扩展名对应的MIME类型.
 */
inline const char* get_mimetype(const std::string& ext) {
    return get_mimetype(ext.c_str());
}

} // namespace util
} // namespace server
} // namespace ic

#endif // IC_SERVER_UTIL_MIME_H_
