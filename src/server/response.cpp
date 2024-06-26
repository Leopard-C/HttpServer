#include "server/response.h"
#include "server/http_cookie.h"
#include "server/http_server.h"
#include "server/logger.h"
#include "server/util/mime.h"
#include "server/util/path.h"
#include <jsoncpp/json/json.h>

namespace ic {
namespace server {

Response::Response(HttpServer* svr)
    : svr_(svr)
{
}

Response::~Response() {
}

void Response::SetHeader(const std::string& name, const std::string& value) {
    headers_.emplace(name, value);
}

void Response::RemoveHeader(const std::string& name) {
    auto range = headers_.equal_range(name);
    headers_.erase(range.first, range.second);
}

void Response::SetContentType(const std::string& content_type) {
    SetHeader("Content-Type", content_type);
}

void Response::SetCookie(const HttpCookie& cookie) {
    SetHeader("Set-Cookie", cookie.ToString());
}

void Response::RemoveAllCookies() {
    RemoveHeader("Set-Cookie");
}

void Response::SetStringBody(unsigned int status_code) {
    is_file_body_ = false;
    status_code_ = status_code;
    string_body_.clear();
}

void Response::SetStringBody(const std::string& body, const std::string& content_type) {
    SetStringBody(200U, body, content_type);
}

void Response::SetStringBody(unsigned int status_code, const std::string& body, const std::string& content_type) {
    is_file_body_ = false;
    status_code_ = status_code;
    string_body_ = body;
    SetContentType(content_type);
}

void Response::SetJsonBody(const Json::Value& root) {
    return SetJsonBody(200U, root);
}

void Response::SetJsonBody(unsigned int status_code, const Json::Value& root) {
    Json::FastWriter fw;
    fw.emitUTF8();
    fw.omitEndingLineFeed();
    return SetStringBody(status_code, fw.write(root), "application/json; charset=utf-8");
}

bool Response::SetFileBody(const std::string& filename, const std::string& content_type/* = "text/plain"*/) {
    return SetFileBody(200U, filename, content_type);
}

bool Response::SetFileBody(unsigned int status_code, const std::string& filename, const std::string& content_type/* = "text/plain"*/) {
    /* 查看文件是否存在 */
    if (!util::path::is_path_exist(filename)) {
        svr_->logger()->Error(LOG_CTX, "File '%s' is not exist", filename.c_str());
        SetStringBody(404U, "Not Found!", "text/plain");
        return false;
    }

    /* MIME类型 */
    if (content_type.empty()) {
        std::string ext = util::path::get_ext(filename);
        SetContentType(util::get_mimetype(ext));
    }
    else {
        SetContentType(content_type);
    }

    filename_ = filename;
    is_file_body_ = true;
    return true;
}

void Response::SetBadRequest(const std::string& why/* = "Bad Request!"*/) {
    return SetStringBody(400U, why, "text/plain");
}

} // namespace server
} // namespace ic
