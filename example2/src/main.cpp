#include <server/http_server.h>
#include <server/router.h>
#include <server/request.h>
#include <server/response.h>

using namespace ic::server;

int main() {
    // 1. 初始化HTTP服务器
    HttpServerConfig config;
    config.set_address("0.0.0.0", 8099);
    config.set_num_threads(4);                // 4个工作线程
    config.set_reuse_address(true);           // 允许复用地址
    config.set_log_access(true);              // 打印请求日志
    config.set_log_access_verbose(false);     // 不打印详细日志（调试时可开启）
    config.set_tcp_stream_timeout_ms(15000);  // 超时时间15s
    config.set_body_limit(11 * 1024 * 1024);  // 请求内容大小限制11MB
    // 日志，可以继承ILogger实现自定义日志输出类
    std::shared_ptr<ILogger> logger = std::make_shared<ConsoleLogger>(LogLevel::kDebug, LogLevel::kDebug);
    HttpServer svr(config, logger);

    // 2. 注册路由(可以使用普通函数、类的静态函数、lambda函数)
    auto router = svr.router();
    // 2.1 GET请求
    router->AddStaticRoute("/echo", HttpMethod::kGET, [](Request& req, Response& res){
        std::string text = req.GetUrlParam("text");
        res.SetStringBody(text, "text/plain");
    });
    // 2.2 POST和OPTIONS请求
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
    // 2.3 正则路由，响应文件
    router->AddRegexRoute("/img/(.*)", HttpMethod::kGET, [](Request& req, Response& res){
        std::string uri = req.GetRouteRegexMatch(0);
        std::string filename = HttpServer::GetBinDirUtf8() + "../data/web/img/" + uri;
        res.SetFileBody(filename);
    });
    // 2.4 响应application/json
    router->AddStaticRoute("/server/time", HttpMethod::kGET, [](Request& req, Json::Value& res){
        res["code"] = 0;
        res["msg"] = "OK";
        res["data"]["time"] = time(NULL);
    });
    // 2.5 关闭服务器
    router->AddStaticRoute("/server/stop", HttpMethod::kGET, [](Request& req, Json::Value& res){
        req.svr()->Stop();
        res["code"] = 0;
        res["msg"] = "OK";
    });

    // 3. 启动服务器
    if (svr.Listen()) {
        svr.Start();
    }

    return 0;
}