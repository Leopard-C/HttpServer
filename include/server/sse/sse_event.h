#ifndef IC_SERVER_SSE_SSE_EVENT_H_
#define IC_SERVER_SSE_SSE_EVENT_H_
#include <string>
#include <vector>
#include <jsoncpp/json/value.h>

namespace ic {
namespace server {

/**
 * @brief SSE事件.
 *
 * @example
 *    retry: 10000
 *    event: user-online
 *    id: 20250719101503001
 *    : This is a line of comment
 *    data: { "id": "001", "name": "Leopard-C" }
 */
class SseEvent {
public:
    SseEvent() = default;
    SseEvent(const std::string& event_name);

    uint32_t retry() const { return retry_; }
    const std::string& type() const { return type_; }
    const std::string& id() const { return id_;  }
    const std::vector<std::pair<std::string, bool>>& body() { return body_; }

    void set_retry(uint32_t retry_ms);
    void set_type(const std::string& type);
    template<typename T>
    void set_id(T id) { set_id(std::to_string(id)); }
    void set_id(const std::string& id);

    void AddComment(const std::string& comment);
    void AddComment(std::string&& comment);
    void AddData(const std::string& data);
    void AddData(std::string&& data);
    void AddJsonData(const Json::Value& json_data);

    std::string Serialize() const;

private:
    uint32_t retry_{0};  // retry: xxx
    std::string type_;    // event: xxx
    std::string id_;      // id: xxx
    std::vector<std::pair<std::string, bool>> body_;   // data or comment (true for data, false for comment)
    size_t body_total_bytes_{0};
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_SSE_SSE_EVENT_H_
