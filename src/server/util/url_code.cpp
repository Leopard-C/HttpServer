#include "server/util/url_code.h"

namespace ic {
namespace server {
namespace util {

inline static unsigned char to_hex(unsigned char x) {
    return  x > 9 ? x + 55 : x + 48;
}

static bool from_hex(unsigned char x, unsigned char* y) {
    if (x >= 'A' && x <= 'Z') {
        *y = x - 'A' + 10;
    }
    else if (x >= 'a' && x <= 'z') {
        *y = x - 'a' + 10;
    }
    else if (x >= '0' && x <= '9') {
        *y = x - '0';
    }
    else {
        return false;
    }
    return true;
}

/**
 * @brief URL编码.
 * 
 * @param  str 输入字符串
 * @return std::string 编码结果
 */
std::string url_encode(const char* str, size_t len) {
    std::string result;
    result.reserve(len);
    for (size_t i = 0; i < len; ++i) {
        if (isalnum((unsigned char)str[i]) || (str[i] == '-') || (str[i] == '_') || (str[i] == '.') || (str[i] == '~')) {
            result += str[i];
        }
        else if (str[i] == ' ') {
            result += '+';
        }
        else {
            result += '%';
            result += to_hex((unsigned char)str[i] >> 4);
            result += to_hex((unsigned char)str[i] % 16);
        }
    }
    return result;
}

/**
 * @brief URL解码.
 * 
 * @param[in]  str 输入字符串
 * @param[out] result 解码结果
 * @retval true  成功
 * @retval false 失败
 */
bool url_decode(const char* str, size_t len, std::string* ori) {
    ori->clear();
    ori->reserve(len);
    for (size_t i = 0; i < len; ++i) {
        if (str[i] == '+') {
            *ori += ' ';
        }
        else if (str[i] == '%') {
            if (i + 2 >= len) {
                return false;
            }
            unsigned char high, low;
            if (!from_hex((unsigned char)str[++i], &high)) {
                return false;
            }
            if (!from_hex((unsigned char)str[++i], &low)) {
                return false;
            }
            *ori += (high*16 + low);
        }
        else {
            *ori += str[i];
        }
    }
    return true;
}

} // namespace util
} // namespace server
} // namespace ic
