#include "server/content_type.h"
#include "server/util/convert/convert_case.h"
#include <vector>

namespace ic {
namespace server {

void ContentType::Parse(StringView content_type) {
    // Content-Type: text/plain
    // Content-Type: application/json; charset=utf-8
    // Content-Type: multipart/form-data; boundary=--------4415942029841
    size_t semicolon = content_type.Find(";", 1);
    set_type(content_type.SubStr(0, semicolon).Trim().ToString());
    if (semicolon == std::string::npos) {
        return;
    }

    /* 分割字符串 */
    std::vector<StringView> keys, values;
    StringView key, value;
    do {
        size_t start = content_type.FindFirstNotOf(semicolon + 1, " ", 1);
        if (start == std::string::npos) {
            break;
        }
        semicolon = content_type.Find(start, ";", 1);
        size_t equal_sign = content_type.Find(start, semicolon, "=", 1);
        if (equal_sign == std::string::npos) {
            break;
        }
        key = content_type.SubStr(start, equal_sign - start).Trim();
        value = content_type.SubStr(equal_sign + 1, semicolon - equal_sign - 1).Trim();
        keys.push_back(key);
        values.push_back(value);
    } while (semicolon != std::string::npos);

    /* 提取charset, boundary */
    for (size_t i = 0; i < keys.size(); ++i) {
        if (keys[i].CompareNoCase("charset") == 0) {
            set_charset(values[i].ToString());
        }
        else if (keys[i].CompareNoCase("boundary") == 0) {
            set_boundary(values[i].ToString());
        }
        else {
            // ignore
        }
    }
}

void ContentType::set_type(const std::string& type) {
    type_ = type;
    util::conv::to_lower(type_);
    /* 检查并更新内置类型. */
    if (type_ == "application/json") {
        built_in_type_ = BuiltInContentType::kApplicationJson;
    }
    else if (type_ == "application/x-www-form-urlencoded") {
        built_in_type_ = BuiltInContentType::kApplicationXWwwFormUrlEncoded;
    }
    else if (type_ == "multipart/form-data") {
        built_in_type_ = BuiltInContentType::kMultipartFormData;
    }
    else if (type_ == "text/plain") {
        built_in_type_ = BuiltInContentType::kTextPlain;
    }
    else if (type_ == "application/xml") {
        built_in_type_ = BuiltInContentType::kApplicationXml;
    }
    else if (type_ == "text/xml") {
        built_in_type_ = BuiltInContentType::kTextXml;
    }
    else {
        built_in_type_ = BuiltInContentType::kUnknown;
    }
}

void ContentType::set_charset(const std::string& charset) {
    charset_ = charset;
    util::conv::to_lower(charset_);
}

void ContentType::set_boundary(const std::string& boundary) {
    boundary_ = boundary;
}

std::string ContentType::ToString() const {
    std::string str;
    str.reserve(type_.length() + charset_.length() + boundary_.length() + 24);
    str += type_;
    if (charset_.length()) {
        str += "; charset=" + charset_;
    }
    if (boundary_.length()) {
        str += "; boundary=" + boundary_;
    }
    return str;
}

/**
 * @brief 判断是否是某种类型（不区分大小写）.
 */
bool ContentType::Is(const std::string& type) const {
    return type_.length() == type.length() && type_ == util::conv::to_lower_copy(type);
}

bool ContentType::IsTextXml() const {
    return built_in_type_ == BuiltInContentType::kTextXml;
}

bool ContentType::IsTextPlain() const {
    return built_in_type_ == BuiltInContentType::kTextPlain;
}

bool ContentType::IsApplicationJson() const {
    return built_in_type_ == BuiltInContentType::kApplicationJson;
}

bool ContentType::IsApplicationXml() const {
    return built_in_type_ == BuiltInContentType::kApplicationXml;
}

bool ContentType::IsApplicationXWwwFormUrlEncoded() const {
    return built_in_type_ == BuiltInContentType::kApplicationXWwwFormUrlEncoded;
}

bool ContentType::IsMultipartFormData() const {
    return built_in_type_ == BuiltInContentType::kMultipartFormData;
}

} // namespace server
} // namespace ic
