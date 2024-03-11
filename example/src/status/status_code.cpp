#include "status_code.h"

namespace ic {
namespace server {
namespace status {

const char* to_string(status_code status) {
    switch (status) {
        case kAuthFailed:          return "Authorization Failed";
        case kPermissionDenied:    return "Permission Denied";
        case kWrongUserOrPassword: return "Wrong user id or password";

        case kNotFound: return "Not Found";
        case kNotExist: return "Not Exist";

        case kGenericError:  return "ERROR";
        case kDuplicate:     return "Duplicate";
        case kAboutToExpire: return "About to expire";
        case kExpired:       return "Expired";

        default: return "ERROR";
    }
}

} // namespace status
} // namespace server
} // namespace ic
