#ifndef IC_SERVER_MULTIPART_PARSER_H_
#define IC_SERVER_MULTIPART_PARSER_H_
#include <map>
#include <memory>
#include <vector>
#include "server/form_param.h"
#include "server/logger.h"
#include "server/string_view.h"

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
    MultipartParser(const std::string& body, std::shared_ptr<ILogger> logger);
    MultipartParser(StringView body);

    bool Parse(const std::string& boundary, std::multimap<std::string, const FormParam*>* result);

private:
    bool SplitToParts(const std::string& boundary, std::vector<Part>* parts) const;

    bool Parse_Part(const Part& part, FormParam* form_param) const;
    bool Parse_Part_ContentDisposition(StringView header_line, FormParam* form_param) const;
    bool Parse_Part_ContentType(StringView header_line, FormParam* form_param) const;

private:
    const StringView body_;
    std::shared_ptr<ILogger> logger_;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_MULTIPART_PARSER_H_
