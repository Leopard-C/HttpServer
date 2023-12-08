#include "util/io.h"

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
bool read_all(const std::string& filename, std::string* content) {
    FILE* fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        return false;
    }

    /* 获取文件长度 */
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    if (length < 0) {
        fclose(fp);
        return false;
    }
    fseek(fp, 0, SEEK_SET);

    /* 分配空间，一次性读取全部内容 */
    content->resize(length, '\0');
    long rn = (long)fread(&((*content)[0]), 1, length, fp);
    fclose(fp);
    return (rn == length);
}

/**
 * @brief 将数据一次性写入文件，如果存在则覆盖.
 * 
 * @param  filename 文件名
 * @param  content 待写入的数据
 * @param  length 待写入的数据长度
 * @retval true 成功
 * @retval false 失败
 */
bool write_all(const std::string& filename, const char* content, size_t length) {
    FILE* fp = fopen(filename.c_str(), "wb+");
    if (!fp) {
        return false;
    }
    fwrite(content, 1, length, fp);
    fclose(fp);
    return true;
}

} // namespace io
} // namespace util
} // namespace server
} // namespace ic
