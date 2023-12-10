#include "server/util/convert/convert_case.h"

namespace ic {
namespace server {
namespace util {
namespace conv {

/**
 * @brief 将字符串转为大写，原地转换，直接修改输入字符串.
 * 
 * @param  str 输入字符串，原地转换
 * @return std::string 输出转为大写后的字符串
 */
std::string& to_upper(std::string& str) {
    for (char& c : str) {
        c = (c >= 'a' && c <= 'z') ? (c - 32) : c;
    }
    return str;
}

/**
 * @brief 将字符串转为小写，原地转换，直接修改输入字符串.
 * 
 * @param  str 输入字符串，原地转换
 * @return std::string 输出转为小写后的字符串
 */
std::string& to_lower(std::string& str) {
    for (char& c : str) {
        c = (c >= 'A' && c <= 'Z') ? (c + 32) : c;
    }
    return str;
}

/**
 * @brief 将字符串转为大写，返回新的字符串.
 * 
 * @param  str 输入字符串
 * @return std::string 输出转为大写后的字符串
 */
std::string to_upper_copy(const std::string& str) {
    std::string str_tmp = str;
    return to_upper(str_tmp);
}

/**
 * @brief 将字符串转为小写，返回新的字符串.
 * 
 * @param  str 输入字符串
 * @return std::string 输出转为小写后的字符串
 */
std::string to_lower_copy(const std::string& str) {
    std::string str_tmp = str;
    return to_lower(str_tmp);
}

} // namespace conv
} // namespace util
} // namespace server
} // namespace ic
