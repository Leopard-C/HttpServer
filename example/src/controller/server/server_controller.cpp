#include "server_controller.h"
#include <server/http_server.h>
#include <server/helper/helper.h>

namespace controller {

/**
 * @brief 关闭服务器.
 * 
 * @route  /api/Server/Shutdown
 * @method POST
 * @config Authorization(1)
 * @config AdminOnly(1)
 */
void ServerController::Shutdown(Request& req, Response& res) {
    API_INIT();
    req.svr()->Stop();
    RETURN_OK();
}

/**
 * @brief 获取所有的线程信息.
 * 
 * @route  /api/Server/DumpThreadInfos
 * @method POST
 * @config Authorization(1)
 * @config AdminOnly(1)
 */
void ServerController::DumpThreadInfos(Request& req, Response& res) {
    API_INIT();
    data = req.svr()->DumpThreadInfos();
    RETURN_OK();
}

} // namespace controller
