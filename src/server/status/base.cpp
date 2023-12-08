#include "status/base.h"

namespace ic {
namespace server {
namespace status {

const char* to_string(base status) {
    switch (status) {
        case base::kNoError: return "OK";

        case base::kInvalidPath:   return "Invalid request path";
        case base::kInvalidMethod: return "Invalid http method";

        case base::kAuthFailed:          return "Authorization Failed";
        case base::kPermissionDenied:    return "Permission Denied";
        case base::kWrongUserOrPassword: return "Wrong user id or password";

        case base::kMissingParam: return "Missing param";
        case base::kInvalidParam: return "Invalid param";

        case base::kRiskControl:
        case base::kRiskControlLv1:
        case base::kRiskControlLv2:
        case base::kRiskControlLv3:
        case base::kRiskControlLv4:
        case base::kRiskControlLv5: return "Trigger the risk control system";
        case base::kQpsLimit:       return "Request too frequently";

        case base::kInternalServerError: return "Internal Server Error";

        case base::kNotFound: return "Not Found";
        case base::kNotExist: return "Not Exist";

        case base::kGenericError:  return "ERROR";
        case base::kDuplicate:     return "Duplicate";
        case base::kAboutToExpire: return "About to expire";
        case base::kExpired:       return "Expired";

        default: return "ERROR";
    }
}

} // namespace status
} // namespace server
} // namespace ic
