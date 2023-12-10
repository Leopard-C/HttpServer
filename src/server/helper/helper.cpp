#include "server/helper/helper.h"

namespace ic {
namespace server {
namespace helper {

void prepare_json_response(Json::Value& root, int code, const std::string& msg, Response& res) {
    root["code"] = code;
    root["msg"] = msg;
    if (root["data"].isNull()) {
        root.removeMember("data");
    }
    res.SetJsonBody(root);
}

} // namespace helper
} // namespace server
} // namespace ic
