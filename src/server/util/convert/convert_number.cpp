#include "server/util/convert/convert_number.h"

namespace ic {
namespace server {
namespace util {
namespace conv {

namespace _internal {

/**
 * @brief 去除首尾的空白字符.
 * 
 * @param[in]  str 输入字符串
 * @param[out] start 左起第一个非空白字符的位置(如果是第一个位置是正/负号，则跳过)，
 * @param[out] end 右起第一个非空白字符的位置
 * @param[out] is_negative 是否是负号开头
 * @return 去除后字符串是否有效(非空)
 */
bool trim(const std::string& str, int* start, int* end, bool* is_negative) {
    const int len = static_cast<int>(str.length());
    if (len == 0) {
        return false;
    }
    /* 找到左起第一个和右起第一个非空白字符的位置 */
    int i = 0, j = len - 1;
    while (i < len && (str[i] == ' ' || str[i] == '\t')) {
        ++i;
    }
    if (i == len) {
        return false;
    }
    while (j >= i && (str[j] == ' ' || str[j] == '\t')) {
        --j;
    }
    /* 裁剪字符串，同时判断是否以正/负号开头 */
    if (str[i] == '-' || str[i] == '+') {
        if (i == j) {
            return false; /* 只有一个正/负号，无效 */
        }
        *start = i + 1;
        *end = j;
    }
    else {
        *start = i;
        *end = j;
    }
    *is_negative = (str[i] == '-');
    return true;
}

/**
 * @brief 是否是合法的浮点数格式.
 * 
 * @param[in]  str 输入字符串
 * @param[in]  start 开始检测的起始位置
 * @param[in]  end 开始检测的结束位置
 * @param[out] err_msg 出错信息
 */
bool is_valid_floating_point(const std::string& str, const int start, const int end) {
    bool has_e = false;
    bool has_dot = false;
    bool has_num_before_e = false;
    bool has_num_after_e = false;
    for (int i = start; i <= end; ++i) {
        switch (str[i]) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': {
            if (has_e) {
                has_num_after_e = true;
            }
            else {
                has_num_before_e = true;
            }
            break;
        }
        case 'e':
        case 'E': {
            /* e只能出现一次，且前面必须有数字 */
            if (has_e || !has_num_before_e)  {
                return false;
            }
            /* e之后可以紧接着一个正/负号 */
            if (i+1 <= end && (str[i+1] == '+' || str[i+1] == '-')) {
                ++i;
            }
            has_e = true;
            break;
        }
        case '.': {
            /* 小数点只能出现一次，且只能出现在e之前 */
            if (has_dot || has_e) {
                return false;
            }
            has_dot = true;
            break;
        }
        default: {
            return false;
        }
        }
    }
    if (!has_num_before_e) {
        return false;
    }
    if (has_e && !has_num_after_e) {
        return false;
    }
    return true;
}

} // namespace _internal

} // namespace conv
} // namespace util
} // namespace server
} // namespace ic
