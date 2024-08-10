/**
 * @file content_type.h
 * @brief 内容类型.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_SERVER_CONTENT_TYPE_H_
#define IC_SERVER_CONTENT_TYPE_H_
#include <string>
#include "string_view.h"

namespace ic {
namespace server {

/**
 * @brief 内容类型.
 */
class ContentType {
public:
    void Parse(StringView content_type);

    const std::string& type() const { return type_;  }
    const std::string& charset() const { return charset_;  }
    const std::string& boundary() const { return boundary_;  }

    void set_type(const std::string& type);
    void set_charset(const std::string& charset);
    void set_boundary(const std::string& boundary);

    std::string ToString() const;

public:
    /**
     * @brief 判断是否是某种类型（不区分大小写）.
     */
    bool Is(const std::string& type) const;

    bool IsTextPlain() const;
    bool IsTextXml() const;

    bool IsApplicationJson() const;
    bool IsApplicationXml() const;
    bool IsApplicationXWwwFormUrlEncoded() const;

    bool IsMultipartFormData() const;

private:
    enum class BuiltInContentType {
        kUnknown = 0,
        kTextPlain = 1,
        kTextXml,
        kApplicationJson,
        kApplicationXml,
        kApplicationXWwwFormUrlEncoded,
        kMultipartFormData
    };

private:
    BuiltInContentType built_in_type_ = BuiltInContentType::kUnknown;
    std::string type_;
    std::string charset_;
    std::string boundary_;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_CONTENT_TYPE_H_
