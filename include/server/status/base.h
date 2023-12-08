#ifndef IC_SERVER_STATUS_BASE_H_
#define IC_SERVER_STATUS_BASE_H_
#include <string>

namespace ic {
namespace server {
namespace status {

/**
 * @brief 基础状态码.
 * 
 * @details 用于返回json格式响应时的code值 { "code": xxx, "msg": "xxx", data: { } }
 * @details 范围 -2, -1, 0, [1000, 1999]
 *
 * @note 可以根据实际需求自定义一套错误代码
 */
enum base {
    kNoError = 0,

    kInvalidPath           = -1,
    kInvalidMethod         = -2,

    /* 登陆、鉴权相关 */
    kAuthFailed            = 1001,
    kPermissionDenied      = 1002,
    kWrongUserOrPassword   = 1003,

    /* 接口参数相关 */
    kMissingParam          = 1101,
    kInvalidParam          = 1102,

    /* 风控系统 */
    kRiskControl           = 1200,
    kRiskControlLv1        = 1201,
    kRiskControlLv2        = 1202,
    kRiskControlLv3        = 1203,
    kRiskControlLv4        = 1204,
    kRiskControlLv5        = 1205,
    kQpsLimit              = 1220,

    /* 服务器内部错误 */
    kInternalServerError   = 1300,

    /* 资源不存在 */
    kNotFound              = 1501,
    kNotExist              = 1502,

    /* 其他通用错误 */
    kGenericError          = 1900,
    kDuplicate             = 1901,
    kAboutToExpire         = 1902,
    kExpired               = 1903
};

const char* to_string(base status);

} // namespace status
} // namespace server
} // namespace ic

#endif // IC_SERVER_STATUS_BASE_H_
