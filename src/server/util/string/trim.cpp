#include "util/string/trim.h"

namespace ic {
namespace server {
namespace util {

std::string& trim(std::string& str, const char* spaces/* = " \r\n\t"*/) {
    if (!str.empty()) {
        str.erase(str.find_last_not_of(spaces) + 1);
        str.erase(0, str.find_first_not_of(spaces));
    }
    return str;
}

std::string& trim_left(std::string& str, const char* spaces/* = " \r\n\t"*/) {
    if (!str.empty()) {
        str.erase(0, str.find_first_not_of(spaces));
    }
    return str;
}

std::string& trim_right(std::string& str, const char* spaces/* = " \r\n\t"*/) {
    if (!str.empty()) {
        str.erase(str.find_last_not_of(spaces) + 1);
    }
    return str;
}

std::string trim_copy(const std::string& str, const char* spaces/* = " \r\n\t"*/) {
    std::string str_tmp = str;
    return trim(str_tmp, spaces);
}

std::string trim_left_copy(const std::string& str, const char* spaces/* = " \r\n\t"*/) {
    std::string str_tmp = str;
    return trim_left(str_tmp, spaces);
}

std::string trim_right_copy(const std::string& str, const char* spaces/* = " \r\n\t"*/) {
    std::string str_tmp = str;
    return trim_right(str_tmp, spaces);
}

} // namespace util
} // namespace server
} // namespace ic
