#pragma once
#include <server/request.h>
#include <server/response.h>

using Request = ic::server::Request;
using Response = ic::server::Response;

class ServerController {
public:
    /**
     * @brief 关闭服务器.
     * 
     * @route  /api/Server/Shutdown
     * @method POST
     * @config Authorization(1)
     * @config AdminOnly(1)
     */
    static void Shutdown(Request& req, Response& res);

    /**
     * @brief 获取监听地址.
     * 
     * @route  /api/Server/GetEndpoints
     * @method POST
     * @config Authorization(1)
     * @config AdminOnly(1)
     */
    static void GetEndpoints(Request& req, Response& res);

    /**
     * @brief 获取服务器快照.
     * 
     * @route  /api/Server/Snapshot
     * @method POST
     * @config Authorization(1)
     * @config AdminOnly(1)
     */
    static void Snapshot(Request& req, Response& res);
};
