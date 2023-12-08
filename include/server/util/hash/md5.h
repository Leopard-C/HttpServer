/**
 * @file md5.h
 * @brief MD5哈希算法.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present Jinbao Chen
 */
#ifndef IC_SERVER_UTIL_HASH_MD5_H_
#define IC_SERVER_UTIL_HASH_MD5_H_
#include <string>

namespace ic {
namespace server {
namespace util {
namespace hash {

/**
 * @brief 计算MD5哈希值，大写字母.
 * 
 * @details 同md5_upper
 * 
 * @param  str 输入字符串
 * @return std::string 长度为32字符的哈希值(大写)
 */
std::string md5(const char* str, size_t length);

inline std::string md5(const std::string& str) {
    return md5(str.c_str(), str.length());
}

/**
 * @brief 计算MD5哈希值，大写字母.
 * 
 * @param  str 输入字符串
 * @param  length 输入字符串长度
 * @return std::string 长度为32字符的哈希值(大写)
 */
std::string md5_upper(const char* str, size_t length);

inline std::string md5_upper(const std::string& str) {
    return md5_upper(str.c_str(), str.length());
}

/**
 * @brief 计算MD5哈希值，小写字母.
 * 
 * @param  str 输入字符串
 * @param  length 输入字符串长度
 * @return std::string 长度为32字符的哈希值(小写)
 */
std::string md5_lower(const char* str, size_t length);

inline std::string md5_lower(const std::string& str) {
    return md5_lower(str.c_str(), str.length());
}

} // namsepace hash
} // namespace util
} // namsepace server
} // namespace ic

#endif // IC_SERVER_UTIL_HASH_MD5_H_
