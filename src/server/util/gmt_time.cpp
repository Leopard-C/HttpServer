#include "server/util/gmt_time.h"

namespace ic {
namespace server {
namespace util {

/**
 * @brief 本地时间，格式化为GMT时间.
 * 
 * @note 示例`Wed, 09 Jun 2021 10:18:14 GMT`
 */
std::string get_gmt_time(time_t time) {
    struct tm tm_gmt;
#ifdef _WIN32
    gmtime_s(&tm_gmt, &time);
#else
    gmtime_r(&time, &tm_gmt);
#endif
    char buf[32] = { 0 };
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm_gmt);
    return buf;
}

} // namespace util
} // namespace server
} // namespace ic
