#include "server/sse/sse_event.h"
#include <jsoncpp/json/writer.h>

namespace ic {
namespace server {

SseEvent::SseEvent(const std::string& event_name) :type_(event_name) {}

void SseEvent::set_retry(uint32_t retry_ms) {
    retry_ = retry_ms;
}

void SseEvent::set_type(const std::string& type) {
    type_ = type;
}

void SseEvent::set_id(const std::string& id) {
    id_ = id;
}

void SseEvent::AddComment(const std::string& comment) {
    const size_t size = comment.size();
    body_.emplace_back(comment, false);
    body_total_bytes_ += size + 3;  // : xxx\n
}

void SseEvent::AddComment(std::string&& comment) {
    const size_t size = comment.size();
    body_.emplace_back(std::move(comment), false);
    body_total_bytes_ += size + 3;  // : xxx\n
}

void SseEvent::AddData(const std::string& data) {
    const size_t size = data.size();
    body_.emplace_back(data, true);
    body_total_bytes_ += size + 7;  // data: xxx\n
}

void SseEvent::AddData(std::string&& data) {
    const size_t size = data.size();
    body_.emplace_back(std::move(data), true);
    body_total_bytes_ += size + 7;  // data: xxx\n
}

void SseEvent::AddJsonData(const Json::Value& json_data) {
    Json::FastWriter fw;
    fw.emitUTF8();
    fw.omitEndingLineFeed();
    return AddData(fw.write(json_data));
}

std::string SseEvent::Serialize() const {
    std::string event_lines;

    if (retry_ > 0) {
        event_lines += "retry: ";
        event_lines += std::to_string(retry_);
        event_lines += "\n";
    }
    if (!type_.empty()) {
        event_lines += "event: ";
        event_lines += type_;
        event_lines += "\n";
    }
    if (!id_.empty()) {
        event_lines += "id: ";
        event_lines += id_;
        event_lines += "\n";
    }

    event_lines.reserve(event_lines.size() + body_total_bytes_ + 1);

    for (const auto& item : body_) {
        if (item.second) {
            event_lines += "data: ";
        }
        else {
            event_lines += ": ";
        }
        event_lines += item.first;
        event_lines += "\n";
    }

    if (!event_lines.empty()) {
        event_lines += "\n";
    }

    return event_lines;
}

} // namespace server
} // namespace ic
