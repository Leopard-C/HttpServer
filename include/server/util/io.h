/**
 * @file io.h
 * @brief 文件IO操作.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present Jinbao Chen
 */
#ifndef IC_SERVER_UTIL_IO_H_
#define IC_SERVER_UTIL_IO_H_
#include <string>

namespace ic {
namespace server {
namespace util {
namespace io {

/**
 * @brief 一次性读取文件所有内容.
 * 
 * @param[in]  filename 文件名
 * @param[out] content 文件内容
 * @retval true 成功
 * @retval false 失败
 */
bool read_all(const std::string& filename, std::string* content);

/**
 * @brief 将数据一次性写入文件，如果存在则覆盖.
 * 
 * @param  filename 文件名
 * @param  content 待写入的数据
 * @param  length 待写入的数据长度
 * @retval true 成功
 * @retval false 失败
 */
bool write_all(const std::string& filename, const char* content, size_t length);

/**
 * @brief 将数据一次性写入文件，如果存在则覆盖.
 * 
 * @param  filename 文件名
 * @param  content 待写入的数据
 * @retval true 成功
 * @retval false 失败
 */
inline bool write_all(const std::string& filename, const std::string& content) {
    return write_all(filename, content.c_str(), content.length());
}

} // namespace io
} // namespace util
} // namespace server
} // namespace ic

#endif // IC_SERVER_UTIL_IO_H_
