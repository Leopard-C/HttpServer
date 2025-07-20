#include "server/sse/sse_message.h"
#include <jsoncpp/json/writer.h>

namespace ic {
namespace server {

static const char* get_event_line_prefix(SseMessage::Type type) {
    switch (type) {
        case SseMessage::Type::kRetry:   return "retry: ";
        case SseMessage::Type::kEvent:   return "event: ";
        case SseMessage::Type::kId:      return "id: ";
        case SseMessage::Type::kData:    return "data: ";
        case SseMessage::Type::kComment: return ": ";
        default:                         return ": ";
    }
}

SseMessage::SseMessage(SseMessage::Type type, const std::string& content) : type(type), content(content) {}

SseMessage::SseMessage(SseMessage&& rhs) noexcept : type(rhs.type), content(std::move(rhs.content)) {}

SseMessage::SseMessage(Type type, const Json::Value& json_content)
    : type(type)
{
    Json::FastWriter fw;
    fw.emitUTF8();
    fw.omitEndingLineFeed();
    fw.write(json_content).swap(content);
}

std::string SseMessage::Serialize() const {
    return get_event_line_prefix(type) + content + "\n";
}

} // namespace server
} // namespace ic
