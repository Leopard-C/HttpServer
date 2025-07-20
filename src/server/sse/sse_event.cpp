#include "server/sse/sse_event.h"

namespace ic {
namespace server {

SseEvent::SseEvent(const std::string& event_name) :name_(event_name) {}

void SseEvent::set_retry(uint32_t retry_ms) {
    retry_ = retry_ms;
}

void SseEvent::set_name(const std::string& event_name) {
    name_ = event_name;
}

void SseEvent::set_id(const std::string& id) {
    id_ = id;
}

void SseEvent::AddComment(const std::string& comment) {
    messages_.emplace_back(SseMessage::Type::kComment, comment);
}

void SseEvent::AddData(const std::string& data) {
    messages_.emplace_back(SseMessage::Type::kData, data);
}

void SseEvent::AddJsonData(const Json::Value& json_data) {
    messages_.emplace_back(SseMessage::Type::kData, json_data);
}

std::string SseEvent::Serialize() const {
    std::string event_lines;
    if (retry_ > 0) {
        event_lines += SseMessage(SseMessage::Type::kRetry, std::to_string(retry_)).Serialize();
    }
    if (!name_.empty()) {
        event_lines += SseMessage(SseMessage::Type::kEvent, name_).Serialize();
    }
    if (!id_.empty()) {
        event_lines += SseMessage(SseMessage::Type::kId, id_).Serialize();
    }

    for (const auto& message : messages_) {
        event_lines += message.Serialize();
    }

    if (event_lines.empty()) {
        event_lines = ": \n";  /* 空事件 */
    }

    event_lines += "\n";
    return event_lines;
}

} // namespace server
} // namespace ic
