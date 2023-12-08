/**
 * @file thread.h
 * @brief 线程相关通用函数.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present Jinbao Chen.
 */
#ifndef IC_SERVER_UTIL_THREAD_H_
#define IC_SERVER_UTIL_THREAD_H_
#include <cstddef>

namespace ic {
namespace server {
namespace util {

/**
 * @brief 获取当前线程ID.
 */
size_t thread_id();

} // namespace util
} // namespace server
} // namespace ic

#endif // IC_SERVER_UTIL_THREAD_H_
