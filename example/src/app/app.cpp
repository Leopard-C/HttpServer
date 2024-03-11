#include "app.h"
#include <server/helper/helper.h>
#include "manager/user_manager.h"
#include "singleton/singleton.h"
#include "status/status_code.h"
#include "../routes.h"

Application::Application() {
}

Application::~Application() {
    Cleanup();
}

bool Application::Init() {
    if (!InitHttpServer()) {
        fprintf(stderr, "Init http server failed\n");
        return false;
    }
    if (!InitRouter()) {
        fprintf(stderr, "Init router failed\n");
        return false;
    }
    return true;
}

void Application::Run() {
    if (server_ && server_->Listen()) {
        server_->Start();
    }
}

void Application::Stop() {
    if (server_) {
        server_->Stop();
    }
}

void Application::Cleanup() {
    ic::Singleton<UserManager>::DesInstance();
    if (server_) {
        delete server_;
        server_ = nullptr;
    }
}

bool Application::InitHttpServer() {
    using namespace ic::server;
    // http server
    std::string cfg_filename = HttpServer::GetBinDir() + "../config/server.json";
    HttpServerConfig config;
    if (!config.ReadFromFile(cfg_filename)) {
        return false;
    }
    /* 日志，可以继承ILogger自定义日志输出类 */
    std::shared_ptr<ILogger> logger = std::make_shared<ConsoleLogger>(LogLevel::kDebug, LogLevel::kWarn);
    server_ = new HttpServer(config, logger);
    server_->set_cb_before_parse_body(BeforeParseBody);
    server_->set_cb_before_handle_request(BeforeHandleRequest);
    server_->set_cb_before_send_response(BeforeSendResponse);

    // singletons
    auto user_mgr = ic::Singleton<UserManager>::Instance();
    if (!user_mgr->Init()) {
        return false;
    }

    return true;
}

bool Application::InitRouter() {
    auto router = server_->router();
    return register_routes(router);
}

/**
 * @brief 请求拦截器：在解析用户URL、headers、cookie等之后，解析用户请求的body内容之前调用.
 */
bool Application::BeforeParseBody(ic::server::Request& req, ic::server::Response& res) {
    auto& config = req.route()->configuration;

    /* 判断是否需要认证Token */
    auto iter = config.find("Authorization");
    if (iter == config.end() || iter->second == "0") {
        /* 无需认证 */
        return true;
    }

    /* 认证Token */
    std::string token = req.GetCookie("Authorization");
    //std::string token = req.GetHeader("Authorization");
    User user;
    auto user_mgr = ic::Singleton<UserManager>::GetInstance();
    if (!user_mgr->Auth(token, &user)) {
        /* 无效的token */
        API_INIT();
        RETURN_CODE(ic::server::status::kAuthFailed) false;
    }

    /* 检查权限(只有管理员有权限访问) */
    iter = config.find("AdminOnly");
    if (iter != config.end() && iter->second == "1") {
        if (user.uid != "admin") {
            /* 权限不足 */
            API_INIT();
            RETURN_CODE(ic::server::status::kPermissionDenied) false;
        }
    }

    /* 将uid写入请求信息中 */
    req.custom_data()["uid"] = user.uid;
    return true;
}

/**
 * @brief 请求拦截器：在解析用户请求的body内容之后，开始处理请求之前调用.
 */
bool Application::BeforeHandleRequest(ic::server::Request& req, ic::server::Response& res) {
    return true;
}

/**
 * @brief 响应拦截器：返回响应内容之前调用.
 */
bool Application::BeforeSendResponse(ic::server::Request& req, ic::server::Response& res) {
    /* 响应头中添加服务器信息 */
    res.SetHeader("Server", "ic Http Server");
    return true;
}
