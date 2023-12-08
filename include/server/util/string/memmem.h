/**
 * @file memmem.h
 * @brief 内存块查找.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present Jinbao Chen.
 */
#ifndef IC_SERVER_UTIL_MEMMEM_H_
#define IC_SERVER_UTIL_MEMMEM_H_
#include <string>

namespace ic {
namespace server {
namespace util {

/**
 * @brief 内存块查找，移植自glibc的代码(windows下没有该函数).
 */
void* memmem(const void* haystack, size_t hs_len, const void* needle, size_t ne_len);

} // namespace util
} // namespace server
} // namespace ic

#endif // IC_SERVER_UTIL_MEMMEM_H_
