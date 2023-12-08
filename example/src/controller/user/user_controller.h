#pragma once
#include <server/request.h>
#include <server/response.h>

namespace controller {

using namespace ic::server;

/**
 * @brief 用户控制器.
 */
class UserController {
public:
    /**
     * @brief 登录.
     * 
     * @route  /api/User/Login
     * @method POST
     * @config Authorization(0)
     */
    static void Login(Request& req, Response& res);

    /**
     * @brief 获取用户信息.
     * 
     * @route  /api/User/GetInfo
     * @method GET,POST
     * @config Authorization(1)
     */
    static void GetInfo(Request& req, Response& res);

    /**
     * @brief 更新用户信息.
     * 
     * @route  /api/User/UpdateInfo
     * @method POST
     * @config Authorization(1)
     */
    static void UpdateInfo(Request& req, Response& res);

    /**
     * @brief 上传用户头像.
     * 
     * @route  /api/User/UploadAvatar
     * @method POST
     * @config Authorization(1)
     */
    static void UploadAvatar(Request& req, Response& res);

    /**
     * @brief 获取头像，返回二进制图片文件.
     * 
     * @route  ~ /api/User/avatar/([a-z0-9]{2})/([a-z0-9]{32}\.(jpg|jpeg|png|gif))
     * @method GET
     * @config Authorization(1)
     */
    static void GetAvatarImage(Request& req, Response& res);
};

} // namespace controller
