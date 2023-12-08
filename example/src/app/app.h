#pragma once
#include <string>
#include <server/http_server.h>

class Application {
public:
    Application();
    ~Application();

    bool Init();
    void Run();
    void Stop();

    ic::server::HttpServer* svr() const { return server_; }

private:
    /**
     * @brief 初始化日志环境.
     */
    bool InitLogger();

    /**
     * @brief 初始化HttpServer.
     */
    bool InitHttpServer();

    /**
     * @brief 初始化路由.
     */
    bool InitRouter();

    /**
     * @brief 清理工作.
     */
    void Cleanup();

    /**
     * @brief 请求拦截器：在解析用户URL和headers之后，解析用户请求的body内容之前调用.
     */
    static bool BeforeParseBody(ic::server::Request& req, ic::server::Response& res);

    /**
     * @brief 请求拦截器：在解析用户请求的body内容之后，开始处理请求之前调用.
     */
    static bool BeforeHandleRequest(ic::server::Request& req, ic::server::Response& res);

    /**
     * @brief 响应拦截器：返回响应内容之前调用.
     */
    static bool BeforeSendResponse(ic::server::Request& req, ic::server::Response& res);

private:
    ic::server::HttpServer* server_{nullptr};
}; // class Application
