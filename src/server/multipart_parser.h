#ifndef IC_SERVER_MULTIPART_PARSER_H_
#define IC_SERVER_MULTIPART_PARSER_H_
#include <map>
#include <vector>
#include "form_item.h"
#include "string_view.h"

namespace ic {
namespace server {

struct Part {
    std::vector<StringView> header_lines;
    StringView content;
};

/**
 * @brief 解析 multipart/form-data 格式的请求体.
 */
class MultipartParser {
public:
    MultipartParser(const std::string& body);
    MultipartParser(StringView body);

    bool Parse(const std::string& boundary, std::multimap<std::string, const FormItem*>* result);

private:
    bool SplitToParts(const std::string& boundary, std::vector<Part>* parts) const;

    bool Parse_Part(const Part& part, FormItem* form_item) const;
    bool Parse_Part_ContentDisposition(StringView header_line, FormItem* form_item) const;
    bool Parse_Part_ContentType(StringView header_line, FormItem* form_item) const;

private:
    const StringView body_;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_MULTIPART_PARSER_H_
