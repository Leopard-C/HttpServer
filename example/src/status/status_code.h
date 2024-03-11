#pragma once
#include <string>

namespace ic {
namespace server {
namespace status {

/**
 * @brief 自定义状态码.
 */
enum status_code {
    /* 登陆、鉴权相关 */
    kAuthFailed            = 1001,
    kPermissionDenied      = 1002,
    kWrongUserOrPassword   = 1003,

    /* 资源不存在 */
    kNotFound              = 1501,
    kNotExist              = 1502,

    /* 其他通用错误 */
    kGenericError          = 1900,
    kDuplicate             = 1901,
    kAboutToExpire         = 1902,
    kExpired               = 1903
};

const char* to_string(status_code status);

} // namespace status
} // namespace server
} // namespace ic
