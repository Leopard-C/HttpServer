#include "user_controller.h"
#include <server/helper/helper.h>
#include <server/http_cookie.h>
#include <server/http_server.h>
#include <server/util/hash/md5.h>
#include <server/util/path.h>
#include <server/util/io.h>
#include "dto/user_dto.h"
#include "manager/user_manager.h"
#include "singleton/singleton.h"

/**
 * @brief 登录.
 * 
 * @route  /api/User/Login
 * @method POST
 * @config Authorization(0)
 */
void UserController::Login(Request& req, Response& res) {
    API_INIT();
    MAKE_DTO(dto::user::LoginDto, dto);

    /* 验证用户名和密码，成功则颁发token */
    auto user_mgr = ic::Singleton<UserManager>::GetInstance();
    std::string token;
    if (!user_mgr->Login(dto.uid, dto.password_md5, 600, &token)) {
        RETURN_CODE(ic::server::status::base::kWrongUserOrPassword);
    }

    /* 将token写入cookie中，之后的请求会自动携带 */
    auto cookie = ic::server::HttpCookie("Authorization", token).Path("/").MaxAge(600).HttpOnly(true);
    res.SetCookie(cookie);

    RETURN_OK();
}

/**
 * @brief 获取用户信息.
 * 
 * @route  /api/User/GetInfo
 * @method GET
 * @config Authorization(1)
 */
void UserController::GetInfo(Request& req, Response& res) {
    API_INIT();

    /* 鉴权时拿到的uid */
    std::string uid = req.custom_data("uid").asString();

    /* 获取用户信息 */
    auto user_mgr = ic::Singleton<UserManager>::GetInstance();
    User user;
    if (!user_mgr->GetUser(uid, &user)) {
        RETURN_INTERNAL_SERVER_ERROR();
    }

    data = user.ToJson();
    data.removeMember("password");
    RETURN_OK();
}

/**
 * @brief 更新用户信息.
 * 
 * @route  /api/User/UpdateInfo
 * @method POST
 * @config Authorization(1)
 */
void UserController::UpdateInfo(Request& req, Response& res) {
    API_INIT();
    MAKE_DTO(dto::user::UpdateInfoDto, dto);

    User user;
    user.uid = req.custom_data("uid").asString();
    dto.SwapToUser(user);

    auto user_mgr = ic::Singleton<UserManager>::GetInstance();
    if (!user_mgr->UpdateUserInfo(user)) {
        RETURN_INTERNAL_SERVER_ERROR_MSG("更新用户信息失败！");
    }

    RETURN_OK();
}

/**
 * @brief 上传用户头像.
 * 
 * @route  /api/User/UploadAvatar
 * @method POST
 * @config Authorization(1)
 */
void UserController::UploadAvatar(Request& req, Response& res) {
    API_INIT();
    using namespace ic::server;

    const FormItem* item = req.GetFormItem("image");
    if (!item) {
        RETURN_MISSING_PARAM("image");
    }
    if (!item->is_file()) {
        RETURN_INVALID_PARAM_MSG("param [image] is not a file object");
    }

    std::string md5 = util::hash::md5_lower(item->content().data(), item->content().size());
    std::string ext = util::path::get_ext(item->filename());
    if (ext.empty()) {
        RETURN_INVALID_PARAM_MSG("filename is invalid");
    }

    /* 写入服务器本地文件 */
    std::string dir = HttpServer::GetBinDir() + "../data/system/avatar/" + md5.substr(0, 2) + "/";
    std::string filename = md5 + ext;
    std::string filepath = dir + filename;
    util::path::create_dir(dir, true);
    if (!util::io::write_all(filepath, item->content().data(), item->content().size())) {
        LError("Write content to file '{}' failed", filepath);
        RETURN_INTERNAL_SERVER_ERROR_MSG("write content to file failed");
    }

    data["avatar"] = filename;
    RETURN_OK();
}

/**
 * @brief 获取头像，返回二进制图片文件.
 * 
 * @route  ~ /api/User/avatar/([a-z0-9]{2})/([a-z0-9]{32}\.(jpg|jpeg|png|gif))
 * @method GET
 * @config Authorization(1)
 */
void UserController::GetAvatarImage(Request& req, Response& res) {
    std::string dir = req.GetRouteRegexMatch(0);
    std::string filename = req.GetRouteRegexMatch(1);
    //std::string ext = req.GetRouteRegexMatch(2);
    std::string filepath = ic::server::HttpServer::GetBinDir() + "../data/system/avatar/" + dir + "/" + filename;
    if (!res.SetFileBody(filepath)) {
        res.SetStringBody(404, "<h2>404 Not Found</h2>", "text/html");
    }
}
