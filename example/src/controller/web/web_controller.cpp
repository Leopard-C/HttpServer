#include "web_controller.h"
#include <server/http_server.h>

/**
 * @brief 访问根目录，跳转/web/index.html.
 * 
 * @route  /
 * @route  /index.html
 * @method GET
 */
void WebController::Index(Request& req, Response& res) {
    req.GetCookie("Authorization");
    res.SetHeader("Location", "web/index.html");
    res.SetStringBody(301);
}

/**
 * @brief 网站图标.
 * 
 * @route  /favicon.ico
 * @method GET
 */
void WebController::Favicon(Request& req, Response& res) {
    std::string filename = ic::server::HttpServer::GetBinDirUtf8() + "../data/web/img/favicon.ico";
    res.SetFileBody(filename);
}

/**
 * @brief 网页资源文件(html/js/css/img).
 * 
 * @route ~ /web/(.+)
 * @method GET
 */
void WebController::GetResource(Request& req, Response& res) {
    std::string uri = req.GetRouteRegexMatch(0);
    std::string filename = ic::server::HttpServer::GetBinDirUtf8() + "../data/web/" + uri;
    res.SetFileBody(filename);
}
