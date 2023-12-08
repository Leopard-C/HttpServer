#include "http_method.h"

namespace ic {
namespace server {

const char* to_string(HttpMethod method) {
    switch (method) {
        case HttpMethod::kGET:  return "GET";
        case HttpMethod::kHEAD: return "HEAD";
        case HttpMethod::kPOST: return "POST";
        case HttpMethod::kPUT:  return "PUT";
        case HttpMethod::kDELETE:  return "DELETE";
        case HttpMethod::kCONNECT: return "CONNECT";
        case HttpMethod::kOPTIONS: return "OPTIONS";
        case HttpMethod::kTRACE:   return "TRACE";
        case HttpMethod::kPATCH:   return "PATCH";
        default: return "NOTSUPPORT";
    }
}

} // namespace server
} // namespace ic
