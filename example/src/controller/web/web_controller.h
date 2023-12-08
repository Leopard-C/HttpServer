#pragma once
#include <server/request.h>
#include <server/response.h>

namespace controller {

using namespace ic::server;

/**
 * @brief web服务器.
 */
class WebController {
public:
    /**
     * @brief 访问根目录，跳转/web/index.html.
     * 
     * @route  /
     * @route  /index
     * @route  /index.html
     * @method GET
     */
    static void Index(Request& req, Response& res);

    /**
     * @brief 网站图标.
     * 
     * @route  /favicon.ico
     * @method GET
     */
    static void Favicon(Request& req, Response& res);

    /**
     * @brief 网页资源文件(html/js/css/img).
     * 
     * @route  ~ /web/(.+)
     * @method GET
     */
    static void GetResource(Request& req, Response& res);
};

} // namespace controller
