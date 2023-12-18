#include "server/helper/helper.h"

namespace ic {
namespace server {
namespace helper {

static void s_default_func_prepare_json_response(Json::Value& root, int code, const std::string& msg, Response& res) {
    root["code"] = code;
    root["msg"] = msg;
    if (root["data"].isNull()) {
        root.removeMember("data");
    }
    res.SetJsonBody(root);
}

FuncPrepareJsonResponse prepare_json_response = s_default_func_prepare_json_response;

/**
 * @brief 自定义接口响应的JSON格式.
 * @details 默认格式: { "code": 0, "msg": "Ok", "data": xxx }
 */
void set_custom_func_prepare_json_response(FuncPrepareJsonResponse func) {
    prepare_json_response = func;
}

} // namespace helper
} // namespace server
} // namespace ic
