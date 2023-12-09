/**
 * @file path.h
 * @brief 程序相关的路径信息.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present Jinbao Chen
 */
#ifndef IC_SERVER_UTIL_PATH_H_
#define IC_SERVER_UTIL_PATH_H_
#include <string>
#include <vector>

namespace ic {
namespace server {
namespace util {
namespace path {

/**
 * @brief 获取可执行文件(完整)路径.
 */
std::string get_bin_filepath();

/**
 * @brief 获取可执行文件名称.
 */
std::string get_bin_filename();

/**
 * @brief 获取可执行文件目录.
 */
std::string get_bin_dir();

/**
 * @brief 格式化目录路径为UNIX格式（正斜杠）.
 * 
 * @details 分隔符全部替换为'/'，且以'/'结尾.
 */
std::string format_dir(const std::string& dir);

/**
 * @brief 格式化目录路径为Windows格式（反斜杠）.
 * 
 * @details 分隔符全部替换为'\'，且以'\'结尾.
 */
std::string format_dir_windows(const std::string& dir);

/**
 * @brief 格式化路径为UNIX格式（正斜杠）.
 */
std::string format_path(const std::string& dir);

/**
 * @brief 格式化路径为Windows格式（反斜杠）.
 */
std::string format_path_windows(const std::string& dir);

/**
 * @brief 创建目录.
 * 
 * @param dir 绝对路径
 * @param parent 如果父级目录不存在，是否创建
 */
bool create_dir(const std::string& dir, bool parent = false);

enum class FilePathPolicy {
    NameOnly = 0,     /* 仅文件名 */
    RelativeDir = 1,  /* 相对路径 */
    FullDir = 2       /* 完整路径 */
};

/**
 * @brief 获取目录下的所有文件名，非递归(不包括子目录).
 * 
 * @param[in]  dir 目录路径
 * @param[out] files 目录下的所有文件名
 * @param[in]  policy 获取的文件路径策略
 */
void list_dir(const std::string& dir, std::vector<std::string>* files, FilePathPolicy policy = FilePathPolicy::FullDir);

/**
 * @brief 获取目录下的所有文件名，递归(包括子目录).
 * 
 * @param[in]  dir 目录路径
 * @param[out] files 目录下的所有文件名
 * @param[in]  policy 获取的文件路径策略
 */
void list_dir_recursive(const std::string& dir, std::vector<std::string>* files, FilePathPolicy policy = FilePathPolicy::FullDir);

/**
 * @brief 文件/目录是否存在.
 */
bool is_path_exist(const std::string& path);

/**
 * @brief 目录是否为空.
 */
bool is_dir_empty(const std::string& dir);

/**
 * @brief 复制文件.
 * 
 * @param filename_src 源文件
 * @param filename_dst 目标文件
 * @param parent 如果目录不存在，是否创建目录
 */
bool copy_file(const std::string& filename_src, const std::string& filename_dst, bool parent = false);

/**
 * @brief 删除文件.
 */
bool remove_file(const std::string& filename);

/**
 * @brief 获取文件的后缀名称.
 */
std::string get_ext(const std::string& filename);

/**
 * @brief 格式化后缀名称.
 * 
 * @details 为空 或者 以'.'开头(.jpg .txt)
 */
std::string format_ext(const std::string& ext);

} // namespace path
} // namespace util
} // namespace server
} // namespace ic

#endif // IC_SERVER_UTIL_PATH_H_
