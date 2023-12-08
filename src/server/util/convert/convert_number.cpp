#include "util/convert/convert_number.h"

namespace ic {
namespace server {
namespace util {
namespace conv {

namespace _internal {

/**
 * @brief 去除首尾的空格.
 * 
 * @param[in]  str 输入字符串
 * @param[out] start 左起第一个非空格的位置(如果是第一个位置是正/负号，则跳过)，
 *                   -1表示字符串中没有有效字符，该字符串不是有效的数字，调用该函数后一定要判断start是否为负 !!!
 * @param[out] end 右起第一个非空格的位置
 * @param[out] is_negative 是否是负号开头
 * @param[out] trimmed 是否去除了首尾的空格字符
 */
void trim(const std::string& str, int* start, int* end, bool* is_negative, bool* trimmed) {
    const int len = static_cast<int>(str.length());
    if (len == 0) {
        *start = *end = -1;
        *is_negative = false;
        *trimmed = false;
        return;
    }
    /* 找到左起第一个和右起第一个非空格的位置 */
    int i = 0, j = 0;
    for (i = 0; i < len; ++i) {
        if (str[i] != ' ') {
            break;
        }
    }
    if (i == len) {
        *start = *end = -1;
        *is_negative = false;
        *trimmed = false;
        return;
    }
    for (j = len - 1; j >= i; --j) {
        if (str[j] != ' ') {
            break;
        }
    }
    /* 裁剪字符串，同时判断是否以负号/正号开头 */
    if (str[i] == '-' || str[i] == '+') {
        if (i == j) {
            *start = *end = -1; /* 只有一个负/负号，无效 */
            *is_negative = false;
            *trimmed = false;
            return;
        }
        else {
            *start = i + 1;
            *end = j;
        }
    }
    else {
        *start = i;
        *end = j;
    }
    *is_negative = (str[i] == '-');
    *trimmed = (i == 0 && j == len - 1);
}

/**
 * @brief 是否是合法的浮点数格式.
 * 
 * @param[in]  str 输入字符串
 * @param[in]  start 开始检测的起始位置
 * @param[in]  end 开始检测的结束位置
 * @param[out] err_msg 出错信息
 * @retval true 成功
 * @retval false 失败
 */
bool is_valid_floating_point(const std::string& str, const int start, const int end) {
    bool has_e = false;
    bool has_dot = false;
    bool has_num_before_e = false;
    bool has_num_after_e = false;
    for (int i = start; i <= end; ++i) {
        switch (str[i]) {
        case 48: case 49: case 50: case 51: case 52:
        case 53: case 54: case 55: case 56: case 57: {
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
            if (has_e || !has_num_before_e)  {
                return false;
            }
            else {
                has_e = true;
            }
            break;
        }
        case '.': {
            if (has_dot || has_e) {
                return false;
            }
            else {
                has_dot = true;
            }
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
