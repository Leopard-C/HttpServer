#ifndef IC_SERVER_SSE_SSE_MESSAGE_H_
#define IC_SERVER_SSE_SSE_MESSAGE_H_
#include <string>
#include <jsoncpp/json/value.h>

namespace ic {
namespace server {

/**
 * @brief SSE消息.
 *
 * @example
 *    retry: 10000
 *    event: user-online
 *    id: 20250719101503001
 *    data: { "id": "001", "name": "Leopard-C" }
 */
class SseMessage {
public:
    enum class Type {
        kRetry   = 1,  // retry: xxx
        kEvent   = 2,  // event: xxx
        kId      = 3,  // id: xxx
        kData    = 4,  // data: xxx
        kComment = 5,  // : xxx
    };

    SseMessage() = default;
    SseMessage(Type type, const std::string& content);
    SseMessage(Type type, const Json::Value& json_content);
    SseMessage(const SseMessage& rhs) = default;
    SseMessage(SseMessage&& rhs) noexcept;

    std::string Serialize() const;

public:
    Type type = Type::kData;
    std::string content;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_SSE_SSE_MESSAGE_H_
