#ifndef IC_SERVER_SSE_SSE_EVENT_H_
#define IC_SERVER_SSE_SSE_EVENT_H_
#include <string>
#include <vector>
#include "sse_message.h"

namespace ic {
namespace server {

/**
 * @brief SSE事件.
 */
class SseEvent {
public:
    SseEvent() = default;
    SseEvent(const std::string& event_name);

    uint32_t retry() const { return retry_; }
    const std::string& name() const { return name_; }
    const std::string& id() const { return id_;  }
    const std::vector<SseMessage>& messages() { return messages_; }

    void set_retry(uint32_t retry_ms);
    void set_name(const std::string& event_name);
    template<typename T>
    void set_id(T id) { set_id(std::to_string(id)); }
    void set_id(const std::string& id);

    void AddComment(const std::string& comment);
    void AddData(const std::string& data);
    void AddJsonData(const Json::Value& json_data);

    std::string Serialize() const;

private:
    uint32_t retry_ = 0;  // retry: xxx
    std::string name_;    // event: xxx
    std::string id_;      // id: xxx
    std::vector<SseMessage> messages_; // comment or data
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_SSE_SSE_EVENT_H_
