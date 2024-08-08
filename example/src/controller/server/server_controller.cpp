#include "server_controller.h"
#include <server/http_server.h>
#include <server/helper/helper.h>

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
    req.svr()->StopAsync();
    RETURN_OK();
}

/**
 * @brief 获取监听地址.
 * 
 * @route  /api/Server/GetEndpoints
 * @method POST
 * @config Authorization(1)
 * @config AdminOnly(1)
 */
void ServerController::GetEndpoints(Request& req, Response& res) {
    API_INIT();
    data.resize(0);
    for (const auto& endpoint : req.svr()->config().endpoints()) {
        Json::Value v_endpoint;
        v_endpoint["ip"] = endpoint.ip;
        v_endpoint["port"] = endpoint.port;
        data.append(v_endpoint);
    }
    RETURN_OK();
}


/**
 * @brief 睡眠几秒钟，模拟耗时请求.
 * 
 * @route  /api/Server/Sleep
 * @method POST
 * @config Authorization(1)
 * @config AdminOnly(1)
 */
void ServerController::Sleep(Request& req, Response& res) {
    API_INIT();
    CHECK_BODY_PARAM_UINT_EX(seconds);
    seconds = seconds <= 15 ? seconds : 15;
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    RETURN_OK();
}

/**
 * @brief 获取服务器快照.
 * 
 * @route  /api/Server/Snapshot
 * @method POST
 * @config Authorization(1)
 * @config AdminOnly(1)
 */
void ServerController::Snapshot(Request& req, Response& res) {
    API_INIT();
    auto snapshot = req.svr()->CreateSnapshot();
    data = snapshot.ToJson();
    RETURN_OK();
}
