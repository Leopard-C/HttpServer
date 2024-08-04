// Generated by script server-assistant.py
// DO NOT EDIT
#include <server/router.h>
#include "controller/server/server_controller.h"
#include "controller/test/test_controller.h"
#include "controller/user/user_controller.h"
#include "controller/web/web_controller.h"

bool register_routes(std::shared_ptr<ic::server::Router> router) {
    using namespace ic::server;
    bool ret = true;

    // controller/server/server_controller.h
    ret &= router->AddStaticRoute("/api/Server/GetEndpoints", HttpMethod::kPOST, ServerController::GetEndpoints, "获取监听地址.", {{"AdminOnly", "1"}, {"Authorization", "1"}});
    ret &= router->AddStaticRoute("/api/Server/Shutdown", HttpMethod::kPOST, ServerController::Shutdown, "关闭服务器.", {{"AdminOnly", "1"}, {"Authorization", "1"}});
    ret &= router->AddStaticRoute("/api/Server/Snapshot", HttpMethod::kPOST, ServerController::Snapshot, "获取服务器快照.", {{"AdminOnly", "1"}, {"Authorization", "1"}});

    // controller/test/test_controller.h
    ret &= router->AddStaticRoute("/api/Test/TestEmpty", HttpMethod::kGET, TestController::TestEmpty, "空请求，用于压力测试.", {{"Authorization", "0"}});
    ret &= router->AddStaticRoute("/api/Test/TestMacro_CHECK_BODY_PARAM", HttpMethod::kPOST, TestController::TestMacro_CHECK_BODY_PARAM, "测试宏-CHECK_BODY_PARAM_xxx.", {});
    ret &= router->AddStaticRoute("/api/Test/TestMacro_CHECK_BODY_PARAM_EX", HttpMethod::kPOST, TestController::TestMacro_CHECK_BODY_PARAM_EX, "测试宏-CHECK_BODY_PARAM_xxx_EX.", {});
    ret &= router->AddStaticRoute("/api/Test/TestMacro_CHECK_JSON_PARAM", HttpMethod::kPOST, TestController::TestMacro_CHECK_JSON_PARAM, "测试宏-CHECK_JSON_PARAM_xxx.", {});
    ret &= router->AddStaticRoute("/api/Test/TestMacro_CHECK_JSON_PARAM_ARRAY", HttpMethod::kPOST, TestController::TestMacro_CHECK_JSON_PARAM_ARRAY, "测试宏-CHECK_JSON_PARAM_xxx_ARRAY.", {});
    ret &= router->AddStaticRoute("/api/Test/TestMacro_CHECK_URL_PARAM", HttpMethod::kGET, TestController::TestMacro_CHECK_URL_PARAM, "测试宏-CHECK_URL_PARAM_xxx.", {});
    ret &= router->AddStaticRoute("/api/Test/TestMacro_GET_BODY_PARAM", HttpMethod::kPOST, TestController::TestMacro_GET_BODY_PARAM, "测试宏-GET_BODY_PARAM_xxx.", {});
    ret &= router->AddStaticRoute("/api/Test/TestMacro_GET_BODY_PARAM_EX", HttpMethod::kPOST, TestController::TestMacro_GET_BODY_PARAM_EX, "测试宏-GET_BODY_PARAM_xxx_EX.", {});
    ret &= router->AddStaticRoute("/api/Test/TestMacro_GET_JSON_PARAM", HttpMethod::kPOST, TestController::TestMacro_GET_JSON_PARAM, "测试宏-GET_JSON_PARAM_xxx.", {});
    ret &= router->AddStaticRoute("/api/Test/TestMacro_GET_JSON_PARAM_ARRAY", HttpMethod::kPOST, TestController::TestMacro_GET_JSON_PARAM_ARRAY, "测试宏-GET_JSON_PARAM_xxx_ARRAY.", {});
    ret &= router->AddStaticRoute("/api/Test/TestMacro_GET_URL_PARAM", HttpMethod::kGET, TestController::TestMacro_GET_URL_PARAM, "测试宏-GET_URL_PARAM_xxx.", {});

    // controller/user/user_controller.h
    ret &= router->AddStaticRoute("/api/User/GetInfo", HttpMethod::kGET | HttpMethod::kPOST, UserController::GetInfo, "获取用户信息.", {{"Authorization", "1"}});
    ret &= router->AddStaticRoute("/api/User/Login", HttpMethod::kPOST, UserController::Login, "登录.", {{"Authorization", "0"}});
    ret &= router->AddStaticRoute("/api/User/UpdateInfo", HttpMethod::kPOST, UserController::UpdateInfo, "更新用户信息.", {{"Authorization", "1"}});
    ret &= router->AddStaticRoute("/api/User/UploadAvatar", HttpMethod::kPOST, UserController::UploadAvatar, "上传用户头像.", {{"Authorization", "1"}});
    ret &= router->AddRegexRoute("/api/User/avatar/([a-z0-9]{2})/([a-z0-9]{32}\\.(jpg|jpeg|png|gif))", HttpMethod::kGET, UserController::GetAvatarImage, "获取头像，返回二进制图片文件.", {{"Authorization", "1"}});

    // controller/web/web_controller.h
    ret &= router->AddStaticRoute("/", HttpMethod::kGET, WebController::Index, "访问根目录，跳转/web/index.html.", {});
    ret &= router->AddStaticRoute("/index", HttpMethod::kGET, WebController::Index, "访问根目录，跳转/web/index.html.", {});
    ret &= router->AddStaticRoute("/index.html", HttpMethod::kGET, WebController::Index, "访问根目录，跳转/web/index.html.", {});
    ret &= router->AddStaticRoute("/favicon.ico", HttpMethod::kGET, WebController::Favicon, "网站图标.", {});
    ret &= router->AddRegexRoute("/web/(.+)", HttpMethod::kGET, WebController::GetResource, "网页资源文件(html/js/css/img).", {});

    return ret;
} // end register_routes