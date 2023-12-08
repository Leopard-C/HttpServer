/**
 * @file http_method.h
 * @brief HTTP请求方法.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_SERVER_HTTP_METHOD_H_
#define IC_SERVER_HTTP_METHOD_H_
#include <string>

namespace ic {
namespace server {

/**
 * @brief HTTP请求方法.
 */
enum HttpMethod {
    kNotSupport = 0,
    kGET  = 1 << 0,
    kHEAD = 1 << 1,
    kPOST = 1 << 2,
    kPUT  = 1 << 3,
    kDELETE  = 1 << 4,
    kCONNECT = 1 << 5,
    kOPTIONS = 1 << 6,
    kTRACE   = 1 << 7,
    kPATCH   = 1 << 8
};

const char* to_string(HttpMethod method);

} // namespace server
} // namespace ic

#endif // IC_SERVER_HTTP_METHOD_H_
