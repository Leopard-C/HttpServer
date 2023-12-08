#include <log/logger.h>
#include <server/http_server.h>
#include <server/router.h>
#include <server/request.h>
#include <server/response.h>

using namespace ic::server;

int main() {
    // 1. 初始化日志
    ic::log::LoggerConfig logger_config;
    if (!logger_config.ReadFromFile(HttpServer::GetBinDir() + "../config/log.ini")) {
        return 1;
    }
    ic::log::Logger::SetConfig(logger_config);

    // 2. 初始化HTTP服务器
    HttpServerConfig server_config;
    if (!server_config.ReadFromFile(HttpServer::GetBinDir() + "../config/server.json")) {
        return 1;
    }
    HttpServer svr(server_config);

    // 3. 注册路由(可以使用普通函数、类的静态函数、lambda函数)
    auto router = svr.router();
    // 3.1 GET请求
    router->AddStaticRoute("/echo", HttpMethod::kGET, [](Request& req, Response& res){
        std::string text = req.GetUrlParam("text");
        res.SetStringBody(text, "text/plain");
    });
    // 3.2 POST和OPTIONS请求
    router->AddStaticRoute("/user/register", HttpMethod::kPOST | HttpMethod::kOPTIONS, [](Request& req, Response& res){
        if (req.method() == HttpMethod::kOPTIONS) {
            res.SetHeader("Access-Control-Allow-Origin", "*");
            return res.SetStringBody(204);
        }
        std::string username = req.GetBodyParam("username");
        std::string password = req.GetBodyParam("password");
        // ...
        res.SetStringBody("OK", "text/plain");
    });
    // 3.3 正则路由，响应文件
    router->AddRegexRoute("/img/(.*)", HttpMethod::kGET, [](Request& req, Response& res){
        std::string uri = req.GetRouteRegexMatch(0);
        std::string filename = HttpServer::GetBinDir() + "../data/web/img/" + uri;
        res.SetFileBody(filename);
    });
    // 3.4 响应application/json
    router->AddStaticRoute("/server/time", HttpMethod::kGET, [](Request& req, Json::Value& res){
        res["code"] = 0;
        res["msg"] = "OK";
        res["data"]["time"] = time(NULL);
    });
    // 3.5 关闭服务器
    router->AddStaticRoute("/server/stop", HttpMethod::kGET, [](Request& req, Json::Value& res){
        req.svr()->Stop();
        res["code"] = 0;
        res["msg"] = "OK";
    });

    // 4. 启动服务器
    if (svr.Listen()) {
        svr.Start();
    }

    return 0;
}