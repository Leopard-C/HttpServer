/**
 * @file http_cookie.h
 * @brief HTTP Cookie.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_SERVER_HTTP_COOKIE_H_
#define IC_SERVER_HTTP_COOKIE_H_
#include <string>

namespace ic {
namespace server {

class HttpCookie {
public:
    enum class SameSiteType {
        Default = 0,
        None = 1,
        Lax = 2,
        Strict = 3
    };

public:
    HttpCookie() = default;
    HttpCookie(const std::string& name, const std::string& value);

    HttpCookie& Name(const std::string& name);
    const std::string& Name() const { return name_; }

    HttpCookie& Value(const std::string& value);
    const std::string& Value() const { return value_; }

    HttpCookie& Path(const std::string& path);
    const std::string& Path() const { return path_; }

    HttpCookie& Domain(const std::string& domain);
    const std::string& Domain() const { return domain_; }

    HttpCookie& Secure(bool secure);
    bool Secure() const { return secure_; }

    HttpCookie& HttpOnly(bool http_only);
    bool HttpOnly() const { return http_only_; }

    HttpCookie& SameSite(SameSiteType same_site);
    SameSiteType SameSite() const { return same_site_; }

    HttpCookie& Expires(time_t expires);
    time_t Expires() const { return expires_; }

    HttpCookie& MaxAge(unsigned long max_age);
    unsigned long MaxAge() const { return max_age_; }

    std::string ToString() const;

private:
    bool secure_{false};
    bool http_only_{false};
    SameSiteType same_site_{SameSiteType::Default};
    time_t expires_{0};
    unsigned long max_age_{0};
    std::string name_;
    std::string value_;
    std::string path_;
    std::string domain_;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_HTTP_COOKIE_H_
