/**
 * @file format_time.h
 * @brief 格式化时间.
 * @author Jinbao Chen (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present Jinbao Chen
 */
#ifndef IC_SERVER_UTIL_FORMAT_TIME_H_
#define IC_SERVER_UTIL_FORMAT_TIME_H_
#include <chrono>
#include <string>

namespace ic {
namespace server {
namespace util {

/**
 * @brief 格式化时间.
 */
std::string format_time(struct tm* tm_, const char* fmt = "%Y-%m-%d %H:%M:%S");

/**
 * @brief 格式化时间.
 */
std::string format_time(time_t tp, const char* fmt = "%Y-%m-%d %H:%M:%S");

/**
 * @brief 格式化时间.
 */
std::string format_time(const std::chrono::system_clock::time_point& tp, const char* fmt = "%Y-%m-%d %H:%M:%S");

/**
 * @brief 格式化时间，精确到毫秒.
 * 
 * @details 例如：2021-10-10 20:08:08.123
 */
std::string format_time_ms(const std::chrono::system_clock::time_point& tp, const char* fmt = "%Y-%m-%d %H:%M:%S");

/**
 * @brief 格式化时间，精确到微秒.
 * 
 * @details 例如：2021-10-10 20:08:08.123456
 */
std::string format_time_us(const std::chrono::system_clock::time_point& tp, const char* fmt = "%Y-%m-%d %H:%M:%S");

/**
 * @brief 格式化时间长度 (自动选择合适的单位).
 */
std::string format_duration(std::chrono::nanoseconds duration);

} // namsspace util
} // namsspace server
} // namespace ic

#endif // IC_SERVER_UTIL_FORMAT_TIME_H_
