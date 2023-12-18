#ifndef IC_SERVER_HELPER_H_
#define IC_SERVER_HELPER_H_
#include <functional>
#include <jsoncpp/json/value.h>
#include <log/logger.h>
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
    RETURN_CODE(ic::server::status::base::kNoError)
#define RETURN_OK_MSG(_msg) \
    RETURN_CODE_MSG(ic::server::status::base::kNoError, _msg)

#define RETURN_NOT_FOUND() \
    RETURN_CODE(ic::server::status::base::kNotFound)
#define RETURN_NOT_FOUND_MSG(_msg) \
    RETURN_CODE_MSG(ic::server::status::base::kNotFound, _msg)

#define RETURN_INTERNAL_SERVER_ERROR() \
    RETURN_CODE(ic::server::status::base::kInternalServerError)
#define RETURN_INTERNAL_SERVER_ERROR_MSG(_msg) \
    RETURN_CODE_MSG(ic::server::status::base::kInternalServerError, _msg)

#define RETURN_ERROR() \
    RETURN_CODE(ic::server::status::base::kGenericError)

#define RETURN_ERROR_MSG(_msg) \
    RETURN_CODE_MSG(ic::server::status::base::kGenericError, _msg)

#define RETURN_INVALID_PARAM(name) \
    RETURN_CODE_MSG(ic::server::status::base::kInvalidParam, std::string("Invalid param:[") + name + std::string("]"))
#define RETURN_INVALID_PARAM_MSG(_msg) \
    RETURN_CODE_MSG(ic::server::status::base::kInvalidParam, _msg)

#define RETURN_MISSING_PARAM(name) \
    RETURN_CODE_MSG(ic::server::status::base::kMissingParam, std::string("Missing param:[") + name + std::string("]"))
#define RETURN_MISSING_PARAM_MSG(_msg) \
    RETURN_CODE_MSG(ic::server::status::base::kMissingParam, _msg)

// for DEBUG
#define DBG_LINE LTrace("LINE: {}", __LINE__);

#define WONT_GET_HERE() LCritical("Won't get here")

#endif // IC_SERVER_HELPER_H_
