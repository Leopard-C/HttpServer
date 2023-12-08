/**
 * @file convert_case.h
 * @brief 字母大小写转换.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present Jinbao Chen.
 */
#ifndef IC_SERVER_UTIL_CONVERT_CASE_H_
#define IC_SERVER_UTIL_CONVERT_CASE_H_
#include <string>

namespace ic {
namespace server {
namespace util {
namespace conv {

/**
 * @brief 字符转为大写，原地转换，直接修改输入的字符.
 * 
 * @param  ch 输入字符，原地转换
 * @return char& 原输入字符的引用
 */
inline char& to_upper(char& ch) {
    ch = (ch >= 'a' && ch <= 'z') ? ch - 32 : ch;
    return ch;
}

/**
 * @brief 字符转为小写，原地转换，直接修改输入的字符.
 * 
 * @param  ch 输入字符，原地转换
 * @return char& 原输入字符的引用
 */
inline char& to_lower(char& ch) {
    ch = (ch >= 'A' && ch <= 'Z') ? ch + 32 : ch;
    return ch;
}

/**
 * @brief 字符转为大写，返回新的字符.
 * 
 * @param  ch 输入字符
 * @return char 输出的大写字符
 */
inline char to_upper_copy(char ch) {
    return (ch >= 'a' && ch <= 'z') ? ch - 32 : ch;
}

/**
 * @brief 字符转为小写，返回新的字符.
 * 
 * @param  ch 输入字符
 * @return char 输出的小写字符
 */
inline char to_lower_copy(char ch) {
    return (ch >= 'A' && ch <= 'Z') ? ch + 32 : ch;
}

/**
 * @brief 将字符串转为大写，原地转换，直接修改输入字符串.
 * 
 * @param  str 输入字符串，原地转换
 * @return std::string 输出转为大写后的字符串
 */
std::string& to_upper(std::string& str);

/**
 * @brief 将字符串转为小写，原地转换，直接修改输入字符串.
 * 
 * @param  str 输入字符串，原地转换
 * @return std::string 输出转为小写后的字符串
 */
std::string& to_lower(std::string& str);

/**
 * @brief 将字符串转为大写，返回新的字符串.
 * 
 * @param  str 输入字符串
 * @return std::string 输出转为大写后的字符串
 */
std::string to_upper_copy(const std::string& str);

/**
 * @brief 将字符串转为小写，返回新的字符串.
 * 
 * @param  str 输入字符串
 * @return std::string 输出转为小写后的字符串
 */
std::string to_lower_copy(const std::string& str);

} // namespace conv
} // namespace util
} // namespace server
} // namespace ic

#endif // IC_SERVER_UTIL_CONVERT_CASE_H_
