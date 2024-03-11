#ifndef IC_SERVER_HELPER_H_
#define IC_SERVER_HELPER_H_
#include <functional>
#include <jsoncpp/json/value.h>
#include "param_check.h"
#include "param_get.h"
#include "../request.h"
#include "../response.h"
#include "../status/base.h"

namespace ic {
namespace server {
namespace helper {

using FuncPrepareJsonResponse = std::function<void(Json::Value& root, int code, const std::string& msg, Response& res)>;

extern FuncPrepareJsonResponse prepare_json_response;

/**
 * @brief 自定义接口响应的JSON格式.
 * @details 默认格式: { "code": 0, "msg": "Ok", "data": xxx }
 */
void set_custom_func_prepare_json_response(FuncPrepareJsonResponse func);

} // namespace helper
} // namespace server
} // namespace ic

#define API_INIT() \
    Json::Value root;\
    Json::Value& data = root["data"];\
    (void)data

#define RETURN_CODE_MSG(_code, _msg) \
    ic::server::helper::prepare_json_response(root, _code, _msg, res);\
    return
#define RETURN_CODE(_code) \
    RETURN_CODE_MSG(_code, ic::server::status::to_string(_code))

#define RETURN_OK() \
    RETURN_CODE(ic::server::status::kNoError)
#define RETURN_OK_MSG(_msg) \
    RETURN_CODE_MSG(ic::server::status::kNoError, _msg)

#define RETURN_INTERNAL_SERVER_ERROR() \
    RETURN_CODE(ic::server::status::kInternalServerError)
#define RETURN_INTERNAL_SERVER_ERROR_MSG(_msg) \
    RETURN_CODE_MSG(ic::server::status::kInternalServerError, _msg)

#define RETURN_INVALID_PARAM(name) \
    RETURN_CODE_MSG(ic::server::status::kInvalidParam, std::string("Invalid param:[") + name + std::string("]"))
#define RETURN_INVALID_PARAM_MSG(_msg) \
    RETURN_CODE_MSG(ic::server::status::kInvalidParam, _msg)

#define RETURN_MISSING_PARAM(name) \
    RETURN_CODE_MSG(ic::server::status::kMissingParam, std::string("Missing param:[") + name + std::string("]"))
#define RETURN_MISSING_PARAM_MSG(_msg) \
    RETURN_CODE_MSG(ic::server::status::kMissingParam, _msg)

#endif // IC_SERVER_HELPER_H_
