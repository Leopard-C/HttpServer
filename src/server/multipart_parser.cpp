#include "multipart_parser.h"
#include <log/logger.h>

#undef IMD
#define IMD "Invalid multipart data. "

namespace ic {
namespace server {

static constexpr size_t HEADER_LINE_MAX_LEN = 1024;
static const char* STR_ContentDisposition = "content-disposition";
static constexpr size_t STR_LEN_ContentDisposition = 19;
static const char* STR_ContentType = "content-type";
static constexpr size_t STR_LEN_ContentType = 12;

MultipartParser::MultipartParser(const std::string& body)
    : body_{ body.data(), body.size() }
{
}

MultipartParser::MultipartParser(StringView body)
    : body_(body)
{
}

bool MultipartParser::Parse(const std::string& boundary, std::multimap<std::string, const FormItem*>* result) {
    result->clear();

    std::vector<Part> parts;
    if (!SplitToParts(boundary, &parts)) {
        LError(IMD"SplitToParts failed");
        return false;
    }

    for (const auto& part : parts) {
        FormItem* form_item = new FormItem();
        if (!Parse_Part(part, form_item)) {
            delete form_item;
            return false;
        }
        result->emplace(form_item->name(), form_item);
    }

    return true;
}

bool MultipartParser::SplitToParts(const std::string& boundary, std::vector<Part>* parts) const {
    const size_t boundary_len = boundary.length();
    const size_t body_len = body_.length();

    /* body最开头可能有CRLF */
    size_t pos = body_.FindFirstNotOf("\r\n", 2);
    if (pos == std::string::npos) {
        return true;
    }

    /* 以 boundary 开头 */
    if (body_.Compare(pos, boundary.c_str(), boundary_len) != 0 || pos + boundary_len + 1 >= body_len) {
        return false;
    }
    pos += boundary_len;
    if (body_[pos] == '-' && body_[pos+1] == '-') {
        /* 解析完成 */
        return true;
    }
    else if (body_[pos] != '\r' || body_[pos+1] != '\n') {
        return false;
    }

    size_t part_start_pos = pos + 2;
    size_t part_end_pos = 0;

    /* 拆分各个part */
    while (true) {
        size_t line_start_pos = part_start_pos;
        size_t line_end_pos = 0;
        Part part;

        /* 处理header line */
        while (true) {
            line_end_pos = body_.Find(line_start_pos, "\r\n", 2);
            if (line_end_pos == std::string::npos || line_end_pos - line_start_pos > HEADER_LINE_MAX_LEN) {
                return false;
            }
            else if (line_end_pos == line_start_pos) {
                line_start_pos = line_end_pos + 2;
                break;
            }
            part.header_lines.emplace_back(body_.begin() + line_start_pos, line_end_pos - line_start_pos);
            line_start_pos = line_end_pos + 2;
        }

        /* 处理content */
        part_end_pos = body_.Find(line_start_pos, boundary.c_str(), boundary.length());
        if (part_end_pos == std::string::npos || part_end_pos - line_start_pos < 2 || body_[part_end_pos-2] != '\r' || body_[part_end_pos-1] != '\n') {
            return false;
        }
        part.content.Assign(body_.begin() + line_start_pos, part_end_pos - line_start_pos - 2);
        parts->push_back(part);

        /* 判断是否结束，或者准备一个part */
        part_start_pos = part_end_pos + boundary_len;
        if (part_start_pos + 2 >= body_len) {
            return false;
        }
        else if (body_[part_start_pos] == '\r' && body_[part_start_pos+1] == '\n') {
            part_start_pos += 2;
            continue;
        }
        else if (body_[part_start_pos] == '-' && body_[part_start_pos+1] == '-') {
            break;
        }
        else {
            return false;
        }
    } // while

    return true;
}

/**
 * @brief 解析一个part.
 */
bool MultipartParser::Parse_Part(const Part& part, FormItem* form_item) const {
    /*
        ------WebKitFormBoundary7MA4YWxkTrZu0gW
        Content-Disposition: form-data; name="description"

        Sample Description
        ------WebKitFormBoundary7MA4YWxkTrZu0gW
        Content-Disposition: form-data; name="file"; filename="example.txt"
        Content-Type: text/plain

        (这里是文件的内容，可能是二进制数据)
        ------WebKitFormBoundary7MA4YWxkTrZu0gW--
    */

    /* 解析Content-Disposition和Content-Type */
    bool has_content_disposition = false;
    bool has_content_type = false;
    for (const auto& header_line : part.header_lines) {
        if (!has_content_disposition && header_line.CompareNoCase(STR_ContentDisposition, STR_LEN_ContentDisposition) == 0) { // Content-Disposition
            has_content_disposition = true;
            if (!Parse_Part_ContentDisposition(header_line, form_item)) {
                LError(IMD"Parse Content-Disposition failed");
                return false;
            }
        }
        else if (!has_content_type && header_line.CompareNoCase(STR_ContentType, STR_LEN_ContentType) == 0) {  // Content-Type
            has_content_type = true;
            if (!Parse_Part_ContentType(header_line, form_item)) {
                LError(IMD"Parse Content-Type failed");
                return false;
            }
        }
        else {
            LError(IMD"Header line in some parts of multipart/form-data is invalid");
            return false;
        }
    }

    form_item->set_content(part.content);
    return true;
}

/**
 * @brief 解析Content-Disposition.
 */
bool MultipartParser::Parse_Part_ContentDisposition(StringView header_line, FormItem* form_item) const {
    // Content-Disposition: form-data; name="xxx"
    // Content-Disposition: form-data; name="xxx"; filename="yyy"
    // Content-Disposition: form-data; filename="yyy"; name="xxx";
    size_t semicolon = header_line.Find(STR_LEN_ContentDisposition, ";", 1);
    if (semicolon == std::string::npos) {
        return false;
    }

    /* 分割字符串 */
    std::vector<StringView> keys, values;
    StringView key, value;
    size_t start, equal_sign;
    do {
        start = header_line.FindFirstNotOf(semicolon + 1, " ", 1);
        if (start == std::string::npos) {
            break;
        }
        semicolon = header_line.Find(start, ";", 1);
        equal_sign = header_line.Find(start, semicolon, "=", 1);
        if (equal_sign == std::string::npos) {
            return false;
        }
        key = header_line.SubStr(start, equal_sign - start).TrimLeft();
        value = header_line.SubStr(equal_sign + 1, semicolon - equal_sign - 1).TrimRight();
        /**
         * Content-Disposition: form-data; name="xxx"; filename="yyy"
         * 等号两侧不能有空格，双引号必须存在，右侧双引号和分号之间可以有空格
         */
        if (key.empty() || key.back() == ' ' || value.length() < 2 || value.front() != '"' || value.back() != '"') {
            return false;
        }
        keys.push_back(key);
        values.push_back(value.SubStr(1, value.size() - 2)); /* 去除左右双引号 */
    } while (semicolon != std::string::npos);

    /* 提取name、filename */
    for (size_t i = 0; i < keys.size(); ++i) {
        if (keys[i].CompareNoCase("name") == 0) {
            form_item->set_name(values[i].ToString());
        }
        if (keys[i].CompareNoCase("filename") == 0) {
            form_item->set_is_file(true);
            form_item->set_filename(values[i].ToString());
        }
        else {
            // ignore
        }
    }
    if (form_item->name().empty()) {
        LError("Content-Disposition.name can not be empty");
        return false;
    }

    return true;
}

/**
 * @brief 解析Content-Type.
 */
bool MultipartParser::Parse_Part_ContentType(StringView header_line, FormItem* form_item) const {
    // Content-Type: text/plain
    // Content-Type: application/octet-stream
    // Content-Type: text/plain; charset=utf-8
    size_t start = header_line.FindFirstNotOf(STR_LEN_ContentType, " ", 1);
    if (start == std::string::npos || header_line[start] != ':') {
        return false;
    }
    // Content-Type: text/plain; charset=utf-8
    size_t semicolon = header_line.Find(start + 1, ";", 1);
    form_item->set_content_type(header_line.SubStr(start + 1, semicolon - start - 1).Trim().ToString());
    if (semicolon == std::string::npos) {
        return true;
    }

    /* 分割字符串 */
    std::vector<StringView> keys, values;
    StringView key, value;
    size_t equal_sign;
    do {
        start = header_line.FindFirstNotOf(semicolon + 1, " ", 1);
        if (start == std::string::npos) {
            break;
        }
        semicolon = header_line.Find(start, ";", 1);
        equal_sign = header_line.Find(start, semicolon, "=", 1);
        if (equal_sign == std::string::npos) {
            return false;
        }
        key = header_line.SubStr(start, equal_sign - start).TrimLeft();
        value = header_line.SubStr(equal_sign + 1, semicolon - equal_sign - 1).TrimRight();
        /**
         * Content-Type: text/plain; charset=utf-8
         * 等号两侧不能有空格，等号两侧不能为空
         */
        if (key.empty() || key.back() == ' ' || value.empty() || value.front() == ' ') {
            return false;
        }
        keys.push_back(key);
        values.push_back(value);
    } while (semicolon != std::string::npos);

    /* 提取charset */
    for (size_t i = 0; i < keys.size(); ++i) {
        if (keys[i].CompareNoCase("charset") == 0) {
            form_item->set_charset(values[i].ToString());
        }
        else {
            // ignore
        }
    }

    return true;
}

} // namespace server
} // namespace ic
