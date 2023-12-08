#include "http_cookie.h"
#include "util/gmt_time.h"

namespace ic {
namespace server {

HttpCookie::HttpCookie(const std::string& name, const std::string& value)
    : name_(name), value_(value)
{
}

HttpCookie& HttpCookie::Name(const std::string& name) {
    name_ = name;
    return *this;
}

HttpCookie& HttpCookie::Value(const std::string& value) {
    value_ = value;
    return *this;
}

HttpCookie& HttpCookie::Path(const std::string& path) {
    path_ = path;
    return *this;
}

HttpCookie& HttpCookie::Domain(const std::string& domain) {
    domain_ = domain;
    return *this;
}

HttpCookie& HttpCookie::Secure(bool secure) {
    secure_ = secure;
    return *this;
}

HttpCookie& HttpCookie::HttpOnly(bool http_only) {
    http_only_ = http_only;
    return *this;
}

HttpCookie& HttpCookie::SameSite(SameSiteType same_site) {
    same_site_ = same_site;
    return *this;
}

HttpCookie& HttpCookie::Expires(time_t expires) {
    expires_ = expires;
    return *this;
}

HttpCookie& HttpCookie::MaxAge(unsigned long max_age) {
    max_age_ = max_age;
    return *this;
}

std::string HttpCookie::ToString() const {
    if (name_.empty()) {
        return {};
    }

    std::string result;
    result.reserve(name_.length() + value_.length() + path_.length() + domain_.length() + 128);
    result += name_ + '=' + value_;

    if (path_.length()) {
        result += "; Path=" + path_;
    }

    if (domain_.length()) {
        result += "; Domain=" + domain_;
    }

    switch (same_site_) {
    default:
        break;
    case SameSiteType::None:
        result += "; SameSite=None";
        break;
    case SameSiteType::Lax:
        result += "; SameSite=Lax";
        break;
    case SameSiteType::Strict:
        result += "; SameSite=Strict";
        break;
    }

    if (expires_ > 0) {
        result += "; Expires=" + util::get_gmt_time(expires_);
    }

    if (max_age_ > 0) {
        result += "; Max-Age=" + std::to_string(max_age_);
    }

    if (secure_ || same_site_ == SameSiteType::None) {
        result += "; secure";
    }

    if (http_only_) {
        result += "; HttpOnly";
    }

    return result;
}

} // namespace server
} // namespace ic
