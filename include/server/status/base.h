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
 * @details 范围 [-5, 0], 0表示成功，非0表示失败.
 *
 * @note 可以根据实际需求自定义一套错误代码
 */
enum status_code_base {
    kNoError = 0,

    kInvalidPath         = -1,
    kInvalidMethod       = -2,
    kMissingParam        = -3,
    kInvalidParam        = -4,
    kInternalServerError = -5,
};

const char* to_string(status_code_base status);

} // namespace status
} // namespace server
} // namespace ic

#endif // IC_SERVER_STATUS_BASE_H_
