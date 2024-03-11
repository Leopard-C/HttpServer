#include "server/util/path.h"
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
        printf("Won't get here. get_bin_filename() error.\n");
        exit(999);
        return {};
    }
    else {
        return bin_filepath.substr(pos + 1);
    }
}

/**
 * @brief 获取可执行文件目录(以斜杠'/'结尾).
 */
std::string get_bin_dir() {
    std::string bin_filepath = get_bin_filepath();
    auto pos = bin_filepath.rfind('/');
    if (pos == std::string::npos) {
        printf("Won't get here. get_bin_dir() error.\n");
        exit(999);
        return {};
    }
    else {
        return bin_filepath.substr(0, pos + 1);
    }
}

/**
 * @brief 格式化目录路径为UNIX格式（正斜杠）.
 * 
 * @details 分隔符全部替换为'/'，且以'/'结尾.
 */
std::string format_dir(const std::string& dir) {
    std::string _dir = format_path(dir);
    if (!_dir.empty() && _dir.back() != '/') {
        _dir += '/';
    }
    return _dir;
}

/**
 * @brief 格式化路径为UNIX格式（正斜杠）.
 */
std::string format_path(const std::string& path) {
#ifdef _WIN32
    std::string _path = path;
    for (char& c : _path) {
        if (c == '\\') {
            c = '/';
        }
    }
    return _path;
#else
    return path;
#endif
}

/**
 * @brief 格式化路径为Windows格式（反斜杠）.
 */
std::string format_path_windows(const std::string& path) {
#ifdef _WIN32
    std::string _path = path;
    for (char& c : _path) {
        if (c == '/') {
            c = '\\';
        }
    }
    return _path;
#else
    return path;
#endif
}

static inline bool __create_dir(const std::string& dir) {
    if (access(dir.c_str(), 0) != 0) {
#ifdef _WIN32
        return _mkdir(dir.c_str()) == 0;
#else
        return mkdir(dir.c_str(), 0777) == 0;
#endif
    }
    return true;
}

static bool _create_dir(const std::string& dir, bool parent) {
    if (parent) {
        auto pos = dir.find('/', 1);
        while (pos != std::string::npos) {
            if (!__create_dir(dir.substr(0, pos))) {
                return false;
            }
            pos = dir.find('/', pos + 1);
        }
        return true;
    }
    else {
        return __create_dir(dir);
    }
}

/**
 * @brief 创建目录.
 * 
 * @param dir 绝对路径
 * @param parent 如果父级目录不存在，是否创建
 */
bool create_dir(const std::string& dir, bool parent/* = false*/) {
    return _create_dir(format_dir(dir), parent);
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
        std::string _filename_dst = format_path(filename_dst);
        auto pos = _filename_dst.rfind('/');
        if (pos != std::string::npos) {
            if (!_create_dir(_filename_dst.substr(0, pos + 1), true)) {
                return false;
            }
        }
    }

    FILE* fp_src = fopen(filename_src.c_str(), "rb");
    if (!fp_src) {
        return false;
    }
    FILE* fp_dst = fopen(filename_dst.c_str(), "wb+");
    if (!fp_dst) {
        fclose(fp_src);
        return false;
    }

    const size_t BUFF_SIZE = 4096;
    char buff[BUFF_SIZE];
    do {
        size_t n = fread(buff, 1, BUFF_SIZE, fp_src);
        fwrite(buff, 1, n, fp_dst);
    } while (!feof(fp_src));

    fclose(fp_src);
    fclose(fp_dst);
    return true;
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
    int len = static_cast<int>(filename.length());
    for (int i = len - 1; i > -1; --i) {
        if (filename[i] == '.') {
            return filename.substr(i);
        }
#ifdef _WIN32
        else if (filename[i] == '/' || filename[i] == '\\') {
            break;
        }
#else
        else if (filename[i] == '/') {
            break;
        }
#endif
    }
    return {};
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
