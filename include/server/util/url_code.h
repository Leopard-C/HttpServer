/**
 * @file url.h
 * @brief URL编码、解码
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present Jinbao Chen.
 */
#ifndef IC_SERVER_UTIL_URL_CODE_H_
#define IC_SERVER_UTIL_URL_CODE_H_
#include <string>

namespace ic {
namespace server {
namespace util {

/**
 * @brief URL编码.
 * 
 * @param  str 输入字符串
 * @param  len 输入字符串长度
 * @return std::string 编码结果
 */
std::string url_encode(const char* str, size_t len);

/**
 * @brief URL解码.
 * 
 * @param[in]  str 输入字符串
 * @param[in]  len 输入字符串长度
 * @param[out] result 解码结果
 * @retval true  成功
 * @retval false 失败
 */
bool url_decode(const char* str, size_t len, std::string* result);

} // namespace util
} // namespace server
} // namespace ic

#endif // IC_SERVER_UTIL_URL_CODE_H_
