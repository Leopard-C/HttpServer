#include "util/format_time.h"
#include <string.h>

namespace ic {
namespace server {
namespace util {

/**
 * @brief 格式化时间.
 */
std::string format_time(struct tm* tm_, const char* fmt/* = "%Y-%m-%d %H:%M:%S"*/) {
    char tmp[128] = { 0 };
    size_t n = strftime(tmp, 127, fmt, tm_);
    tmp[n] = '\0';
    return tmp;
}

/**
 * @brief 格式化时间.
 */
std::string format_time(time_t tp, const char* fmt/* = "%Y-%m-%d %H:%M:%S"*/) {
    struct tm tm_;
#ifdef _WIN32
    localtime_s(&tm_, &tp);
#else
    localtime_r(&tp, &tm_);
#endif
    return format_time(&tm_, fmt);
}

/**
 * @brief 格式化时间.
 */
std::string format_time(const std::chrono::system_clock::time_point& tp, const char* fmt/* = "%Y-%m-%d %H:%M:%S"*/) {
    return format_time(std::chrono::system_clock::to_time_t(tp), fmt);
}

/**
 * @brief 格式化时间，精确到微秒.
 * 
 * @details 例如：2021-10-10 20:08:08.123456
 */
std::string format_time_us(const std::chrono::system_clock::time_point& tp, const char* fmt/* = "%Y-%m-%d %H:%M:%S"*/)  {
    uint64_t micro_secs = tp.time_since_epoch().count() % 1000000ULL;
    return format_time(tp, fmt) + "." + std::to_string(micro_secs);
}

/**
 * @brief 格式化时间长度 (自动选择合适的单位).
 */
std::string format_duration(std::chrono::nanoseconds duration) {
    uint64_t ns = duration.count();
    if (ns < 1000UL) {
        return std::to_string(ns) + "ns";
    }
    else if (ns < 1000000ULL) {
        return std::to_string(ns / 1000ULL) + "us";
    }
    else if (ns < 1000000000ULL) {
        return std::to_string(ns / 1000000ULL) + "ms";
    }
    else {
        char str[16] = { 0 };
        snprintf(str, sizeof(str), "%.3fs", ns / 1000000000.0);
        return str;
    }
}

} // namespace util
} // namespace server
} // namespace ic
