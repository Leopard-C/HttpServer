#include "util/path.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#  include <windows.h>
#  include <io.h>
#  include <direct.h>
#else
#  include <dirent.h>
#  include <errno.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif

namespace ic {
namespace server {
namespace util {
namespace path {

/**
 * @brief 字符串转为小写
 */
static inline std::string& to_lower(std::string& str) {
    for (char& c : str) {
        if (c >= 'A' && c <= 'Z') {
            c += 32;
        }
    }
    return str;
}

/**
 * @brief 获取可执行文件(完整)路径.
 */
std::string get_bin_filepath() {
    char bin_filename[512] = { 0 };
#ifdef _WIN32
    int n = GetModuleFileNameA(NULL, bin_filename, 511);
    for (int i = 0; i < n; ++i) {
        if (bin_filename[i] == '\\') {
            bin_filename[i] = '/';
        }
    }
#else
    int n = readlink("/proc/self/exe", bin_filename, 511);
    if (n < 0) {
        printf("get exe filename failed. errno: %d. message: %s\n", errno, strerror(errno));
        exit(999);
    }
#endif
    bin_filename[n] = '\0';
    return bin_filename;
}

/**
 * @brief 获取可执行文件名称.
 */
std::string get_bin_filename() {
    std::string bin_filepath = get_bin_filepath();
    auto pos = bin_filepath.rfind('/');
    if (pos == std::string::npos) {
        printf("Won't get here. get_bin_dir error.\n");
        exit(999);
        return {};
    }
    else {
        return bin_filepath.substr(pos + 1);
    }
}

/**
 * @brief 获取可执行文件目录.
 */
std::string get_bin_dir() {
    std::string bin_filepath = get_bin_filepath();
    auto pos = bin_filepath.rfind('/');
    if (pos == std::string::npos) {
        printf("Won't get here. get_bin_dir error.\n");
        exit(999);
        return {};
    }
    else {
        return bin_filepath.substr(0, pos + 1);
    }
}

/**
 * @brief 格式化目录路径.
 * 
 * @details 以'/'结尾.
 */
std::string format_dir(const std::string& dir) {
    size_t len = dir.length();
    std::string _dir = dir;
#ifdef _WIN32
    for (size_t i = 0; i < len; ++i) {
        if (_dir[i] == '\\') {
            _dir[i] = '/';
        }
    }
#endif
    if (!_dir.empty() && _dir[len - 1] != '/') {
        _dir += '/';
    }
    return _dir;
}

static bool _create_dir(const std::string& dir) {
    if (access(dir.c_str(), 0) != 0) {
#ifdef _WIN32
        if (_mkdir(dir.c_str()) != 0) {
            return false;
        }
#else
        if (mkdir(dir.c_str(), 0777) != 0) {
            return false;
        }
#endif
    }
    return true;
}

/**
 * @brief 创建目录.
 * 
 * @param dir 绝对路径
 * @param parent 如果父级目录不存在，是否创建
 */
bool create_dir(const std::string& dir, bool parent/* = false*/) {
    std::string _dir = format_dir(dir);
    auto pos = _dir.find('/', 1);
    if (parent) {
        while (pos != std::string::npos) {
            if (!_create_dir(_dir.substr(0, pos))) {
                return false;
            }
            pos = _dir.find('/', pos + 1);
        }
        return true;
    }
    else {
        return _create_dir(dir);
    }
}

/**
 * @brief 获取目录下的所有文件名，非递归(不包括子目录).
 * 
 * @param[in]  dir 目录路径
 * @param[out] files 目录下的所有文件名
 * @param[in]  policy 获取的文件路径策略
 */
static void _list_dir(const std::string& dir, std::vector<std::string>* files, FilePathPolicy policy) {
#ifdef _WIN32
    _finddata_t file_info;
    intptr_t handle = _findfirst((dir + "*.*").c_str(), &file_info);
    if (handle == -1) {
        return;
    }
    do {
        if (!(file_info.attrib & _A_SUBDIR)) {
            std::string filename = (policy == FilePathPolicy::FullDir) ? dir + file_info.name : file_info.name;
            files->emplace_back(filename);
        }
    } while (_findnext(handle, &file_info) == 0);
    _findclose(handle);
#else
    DIR* _dir = opendir(dir.c_str());
    if (!_dir) {
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(_dir)) != NULL) {
        if (entry->d_type != DT_DIR) {
            std::string filename = (policy == FilePathPolicy::FullDir) ? dir + entry->d_name : entry->d_name;
            files->emplace_back(filename);
        }
    }
    closedir(_dir);
#endif // _WIN32
}

/**
 * @brief 获取目录下的所有文件名，非递归(不包括子目录).
 * 
 * @param[in]  dir 目录路径
 * @param[out] files 目录下的所有文件名
 * @param[in]  policy 获取的文件路径策略
 */
void list_dir(const std::string& dir, std::vector<std::string>* files, FilePathPolicy policy/* = FilePathPolicy::FullDir*/) {
    files->clear();
    _list_dir(format_dir(dir), files, policy);
}

/**
 * @brief 获取目录下的所有文件名，递归(包括子目录).
 * 
 * @param[in]  dir 目录路径
 * @param[in]  root_dir_len 根路径长度
 * @param[out] files 目录下的所有文件名
 * @param[in]  policy 获取的文件路径策略
 */
static void _list_dir_recursive(const std::string& dir, const size_t root_dir_len, std::vector<std::string>* files, FilePathPolicy policy) {
#ifdef _WIN32
    _finddata_t file_info;
    intptr_t handle = _findfirst((dir + "*.*").c_str(), &file_info);
    if (handle == -1) {
        return;
    }
    do {
        /* 子目录 */
        if (file_info.attrib & _A_SUBDIR) {
            if ((strcmp(file_info.name, ".") != 0) && (strcmp(file_info.name, "..") != 0)) {
                std::string sub_dir = dir + file_info.name + '/';
                _list_dir_recursive(sub_dir, root_dir_len, files, policy);
            }
        }
        /* 文件 */
        else {
            if (policy == FilePathPolicy::FullDir) {
                files->push_back(dir + file_info.name);
            }
            else if (policy == FilePathPolicy::RelativeDir) {
                files->push_back(dir.substr(root_dir_len) + file_info.name);
            }
            else {
                files->push_back(file_info.name);
            }
        }
    } while (_findnext(handle, &file_info) == 0);
    _findclose(handle);
#else
    DIR* _dir = opendir(dir.c_str());
    if (!_dir) {
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(_dir)) != NULL) {
        /* 子目录 */
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                std::string sub_dir = dir + entry->d_name + '/';
                _list_dir_recursive(sub_dir, root_dir_len, files, policy);
            }
        }
        /* 文件 */
        else {
            if (policy == FilePathPolicy::FullDir) {
                files->push_back(dir + entry->d_name);
            }
            else if (policy == FilePathPolicy::RelativeDir) {
                files->push_back(dir.substr(root_dir_len) + entry->d_name);
            }
            else {
                files->push_back(entry->d_name);
            }
        }
    }
    closedir(_dir);
#endif // _WIN32
}

/**
 * @brief 获取目录下的所有文件名，递归(包括子目录).
 * 
 * @param[in]  dir 目录路径
 * @param[out] files 目录下的所有文件名
 * @param[in]  policy 获取的文件路径策略
 */
void list_dir_recursive(const std::string& dir, std::vector<std::string>* files, FilePathPolicy policy/* = FilePathPolicy::FullDir*/) {
    files->clear();
    std::string _dir = format_dir(dir);
    _list_dir_recursive(_dir, _dir.length(), files, policy);
}

/**
 * @brief 文件/目录是否存在.
 */
bool is_path_exist(const std::string& path) {
    return access(path.c_str(), 0) == 0;
}

/**
 * @brief 目录是否为空.
 */
bool is_dir_empty(const std::string& dir) {
#ifdef _WIN32
    _finddata_t file_info;
    intptr_t handle = _findfirst((format_dir(dir) + "*.*").c_str(), &file_info);
    if (handle == -1) {
        return true;
    }
    do {
        if (file_info.attrib & _A_SUBDIR) {
            if ((strcmp(file_info.name, ".") == 0) || (strcmp(file_info.name, "..") == 0)) {
                continue;
            }
        }
        _findclose(handle);
        return false;
    } while (_findnext(handle, &file_info) == 0);
    _findclose(handle);
    return true;
#else
    DIR* _dir = opendir(dir.c_str());
    if (!_dir) {
        return true;
    }
    struct dirent *entry;
    while ((entry = readdir(_dir)) != NULL) {
        /* 子目录 */
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
        }
        closedir(_dir);
        return false;
    }
    closedir(_dir);
    return true;
#endif // _WIN32
}

/**
 * @brief 复制文件.
 * 
 * @param filename_src 源文件
 * @param filename_dst 目标文件
 * @param parent 如果目录不存在，是否创建目录
 */
bool copy_file(const std::string& filename_src, const std::string& filename_dst, bool parent/* = false*/) {
    if (parent) {
        auto pos = filename_dst.rfind('/');
        if (pos != std::string::npos) {
            std::string dir = filename_dst.substr(0, pos + 1);
            if (!create_dir(dir, true)) {
                return false;
            }
        }
    }

    FILE* fp_src = nullptr;
    FILE* fp_dst = nullptr;

    bool success = false;
    do {
        fp_src = fopen(filename_src.c_str(), "rb");
        if (!fp_src) {
            break;
        }
        fp_dst = fopen(filename_dst.c_str(), "wb+");
        if (!fp_dst) {
            break;
        }

        const size_t BUFF_SIZE = 4096;
        char buff[BUFF_SIZE];
        while (!feof(fp_src)) {
            memset(buff, 0, BUFF_SIZE);
            size_t n = fread(buff, 1, BUFF_SIZE, fp_src);
            if (n == 0) {
                break;
            }
            fwrite(buff, 1, n, fp_dst);
        }

        success = true;
    } while (false);

    if (fp_src) {
        fclose(fp_src);
    }
    if (fp_dst) {
        fclose(fp_dst);
    }
    return success;
}

/**
 * @brief 删除文件.
 */
bool remove_file(const std::string& filename) {
    return remove(filename.c_str()) == 0;
}

/**
 * @brief 获取文件的后缀名称.
 */
std::string get_ext(const std::string& filename) {
    std::string ext;
    auto pos = filename.rfind('.');
    if (pos != std::string::npos) {
        ext = filename.substr(pos);
        to_lower(ext);
    }
    return ext;
}

/**
 * @brief 格式化后缀名称.
 * 
 * @details 为空 或者 以'.'开头(.jpg .txt)
 */
std::string format_ext(const std::string& ext) {
    return (ext.empty() || ext[0] == '.') ? ext : "." + ext;
}

} // namespace path
} // namespace util
} // namespace server
} // namespace ic
