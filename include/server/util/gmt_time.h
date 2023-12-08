#ifndef IC_SERVER_UTIL_GMT_TIME_H_
#define IC_SERVER_UTIL_GMT_TIME_H_
#include <string>
#include <time.h>

namespace ic {
namespace server {
namespace util {

/**
 * @brief 本地时间，格式化为GMT时间.
 * 
 * @note 示例`Wed, 09 Jun 2021 10:18:14 GMT`
 */
std::string get_gmt_time(time_t time);

} // namespace util
} // namespace server
} // namespace ic

#endif // IC_SERVER_UTIL_GMT_TIME_H_
