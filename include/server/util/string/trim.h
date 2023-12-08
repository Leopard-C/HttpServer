/**
 * @file trim.h
 * @brief 去除空白字符.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present Jinbao Chen.
 */
#ifndef IC_SERVER_UTIL_STRING_TRIM_H_
#define IC_SERVER_UTIL_STRING_TRIM_H_
#include <string>

namespace ic {
namespace server {
namespace util {

/**
 * @brief 去除空白字符，原地去除.
 * 
 * @param  str 输入字符串
 * @param  spaces 空白字符(数组)
 * @return std::string& 输入字符串的引用
 * @{
 */
std::string& trim(std::string& str, const char* spaces = " \r\n\t");
std::string& trim_left(std::string& str, const char* spaces = " \r\n\t");
std::string& trim_right(std::string& str, const char* spaces = " \r\n\t");
/** @} */

/**
 * @brief 去除空白字符，生成新的字符串.
 * 
 * @param  str 输入字符串
 * @param  spaces 空白字符(数组)
 * @return std::string 去除空白字符后的新的字符串
 * @{
 */
std::string trim_copy(const std::string& str, const char* spaces = " \r\n\t");
std::string trim_left_copy(const std::string& str, const char* spaces = " \r\n\t");
std::string trim_right_copy(const std::string& str, const char* spaces = " \r\n\t");
/** @} */

} // namespace util
} // namespace server
} // namespace ic

#endif // IC_SERVER_UTIL_STRING_TRIM_H_
