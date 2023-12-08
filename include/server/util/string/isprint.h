/**
 * @file isprint.h
 * @brief 字符串是否可打印.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present Jinbao Chen.
 */
#ifndef IC_SERVER_UTIL_ISPRINT_H_
#define IC_SERVER_UTIL_ISPRINT_H_
#include <string>

namespace ic {
namespace server {
namespace util {

/**
 * @brief 字符串是否可打印.
 */
bool isprint(const char* str, size_t len);

inline bool isprint(const std::string& str) {
    return isprint(str.c_str(), str.length());
}

} // namespace util
} // namespace server
} // namespace ic

#endif // IC_SERVER_UTIL_ISPRINT_H_
