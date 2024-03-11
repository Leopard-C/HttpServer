#include "server/status/base.h"

namespace ic {
namespace server {
namespace status {

const char* to_string(status_code_base status) {
    switch (status) {
        case status_code_base::kNoError: return "OK";

        case status_code_base::kInvalidPath:   return "Invalid request path";
        case status_code_base::kInvalidMethod: return "Invalid http method";
        case status_code_base::kMissingParam: return "Missing param";
        case status_code_base::kInvalidParam: return "Invalid param";
        case status_code_base::kInternalServerError: return "Internal Server Error";

        default: return "ERROR";
    }
}

} // namespace status
} // namespace server
} // namespace ic
