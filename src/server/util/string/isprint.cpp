#include "server/util/string/isprint.h"

namespace ic {
namespace server {
namespace util {

/**
 * @brief 字符串是否可打印.
 */
bool isprint(const char* str, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        if (!std::isprint(str[i])) {
            return false;
        }
    }
    return true;
}

} // namespace util
} // namespace server
} // namespace ic
