#include <thread>
#include <server/http_server.h>
#include <server/router.h>
#include <server/request.h>
#include <server/response.h>
#include <server/sse/sse_provider.h>
#include <server/util/convert/convert_number.h>

using namespace ic::server;

int main() {
    std::atomic_uint32_t sse_thread_count_{0};

    // 1. 初始化HTTP服务器
    HttpServerConfig config;
    config.add_endpoint("0.0.0.0", 8099, true);
    config.set_min_num_threads(2);            // 最少2个工作线程
    config.set_max_num_threads(8);            // 最多8个工作线程
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
    // 2.5 模拟耗时请求 (/server/sleep?seconds=5)
    router->AddStaticRoute("/server/sleep", HttpMethod::kGET, [](Request& req, Json::Value& res){
        uint64_t seconds = 5;
        util::conv::convert_number(req.GetUrlParam("seconds"), &seconds);
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        res["code"] = 0;
        res["msg"] = "OK";
    });
    // 2.6 SSE(Server-Sent Event)
    router->AddStaticRoute("/sse", HttpMethod::kGET, [&sse_thread_count_](Request& req, Response& res){
        auto sse_provider = SseProvider::Create();
        sse_provider->set_max_queue_length(128);
        sse_provider->set_heartbeat_interval(500);
        std::thread t([sse_provider, &sse_thread_count_] {
            sse_thread_count_.fetch_add(1);
            int id = 11;
            while (sse_provider->is_alive() && id-- > 0) {
                SseEvent event;
                if (id == 10) {
                    event.set_retry(5000);
                }
                if (id == 0) {
                    event.set_type("bye");
                }
                else {
                    event.set_type("hello");
                    event.set_id(id);
                    event.AddComment("This is a line of comment");
                    event.AddData("Hello world! " + std::to_string(id));
                }
                sse_provider->Push(event);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            sse_provider->Shutdown();
            sse_thread_count_.fetch_sub(1);
        });
        t.detach();
        res.SetSseBody(sse_provider);
    });
    // 2.7 关闭服务器
    router->AddStaticRoute("/server/stop", HttpMethod::kGET, [](Request& req, Json::Value& res){
        req.svr()->StopAsync();
        res["code"] = 0;
        res["msg"] = "OK";
    });

    // 3. 启动服务器
    svr.Start();

    while (sse_thread_count_ > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 0;
}
