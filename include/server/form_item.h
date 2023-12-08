/**
 * @file form_item.h
 * @brief HTTP请求体multipart/form-data.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_SERVER_FORM_ITEM_H_
#define IC_SERVER_FORM_ITEM_H_
#include <string>
#include "string_view.h"

namespace ic {
namespace server {

/**
 * @brief 通过`Content-Type: multipart/form-data`上传的表单中的某项.
 */
class FormItem {
public:
    /**
     * @brief 是否是文件.
     */
    bool is_file() const { return is_file_; }

    /**
     * @brief 字段名（参数名称）.
     */
    const std::string& name() const { return name_; }

    /**
     * @brief 文件名.
     */
    const std::string& filename() const { return filename_; }

    /**
     * @brief 字段值，或文件内容.
     */
    const StringView& content() const { return content_; }

    /**
     * @brief 内容类型.
     */
    const std::string& content_type() const { return content_type_; }

    /**
     * @brief 内容编码格式.
     */
    const std::string& charset() const { return charset_; }

public:
    void set_is_file(bool is_file) { is_file_ = is_file; }
    void set_content(const StringView& content) { content_ = content; }
    void set_name(const std::string& name) { name_ = name; }
    void set_filename(const std::string& filename) { filename_ = filename; }
    void set_content_type(const std::string& content_type) { content_type_ = content_type; }
    void set_charset(const std::string& charset) { charset_ = charset; }

private:
    bool is_file_{false};
    StringView content_;
    std::string name_;
    std::string filename_;
    std::string content_type_;
    std::string charset_;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_FORM_ITEM_H_
