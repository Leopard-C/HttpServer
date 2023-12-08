/**
 * @file xxh64.h
 * @brief XXH64哈希算法.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present Jinbao Chen.
 */
#ifndef IC_SERVER_UTIL_HASH_XXH64_H_
#define IC_SERVER_UTIL_HASH_XXH64_H_
#include <string>

namespace ic {
namespace server {
namespace util {
namespace hash {

/**
 * @brief 计算XXH64哈希值.
 * 
 * @param  str 输入字符串
 * @param  len 字符串长度
 * @param  seed 种子
 * @return uint64_t 哈希值
 */
uint64_t xxh64(const char* str, const size_t len, const uint64_t seed);

/**
 * @brief 计算XXH64哈希值.
 * 
 * @param  str 输入字符串
 * @param  seed 种子
 * @return uint64_t 哈希值
 */
inline uint64_t xxh64(const std::string& str, const uint64_t seed) {
    return xxh64(str.c_str(), str.length(), seed);
}

} // namespace hash
} // namespace util
} // namespace server
} // namespace ic

#endif // IC_SERVER_UTIL_HASH_XXH64_H_
