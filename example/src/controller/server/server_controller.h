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
     * @brief 获取所有线程信息.
     * 
     * @route  /api/Server/DumpThreadInfos
     * @method POST
     * @config Authorization(1)
     * @config AdminOnly(1)
     */
    static void DumpThreadInfos(Request& req, Response& res);
};
