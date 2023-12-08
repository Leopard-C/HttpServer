# Http Server

基于 `Boost.Beast` 实现的跨平台、高性能、开箱即用的HTTP服务器。

## 1. 依赖

+ C++11
+ Boost 1.70.0
+ [Boost.Asio](https://github.com/boostorg/asio), [Boost.Beast](https://github.com/boostorg/beast), [Boost.Regex](https://github.com/boostorg/regex)（可选，性能比 std::regex 性能高些）
+ [Leopard-C/jsoncpp](https://github.com/Leopard-C/jsoncpp) （修改自[open-source-parsers/jsoncpp](https://github.com/open-source-parsers/jsoncpp)）
+ [gabime/spdlog](https://github.com/gabime/spdlog)
+ [Leopard-C/spdlog-wrapper](https://github.com/Leopard-C/spdlog-wrapper)


## 2. 功能要点

+ 支持`Linux`, `Windows`平台
+ 支持`GET`,`HEAD`,`POST`,`PUT`,`DELETE`,`CONNECT`,`OPTIONS`,`TRACE`,`PATCH`方法
+ 请求拦截器
+ 响应拦截器
+ 路由管理(静态路由，正则路由)
+ 工作线程数量设置
+ `Keep-Alive`超时时间设置
+ `body`内容大小限制
+ (大)文件响应
+ 支持`Set-Cookie`
+ 自动解析以下3种类型的body
    + `application/x-www-form-urlencoded`
    + `application/json`
    + `multipart/form-data`

## 2. 编译静态库、示例程序

> 编译安装 `Boost` (如果已安装请跳过)
```shell
# 安装依赖项 (Ubuntu)
sudo apt install build-essential g++ autotools-dev libicu-dev build-essential python-dev libbz2-dev
# 安装依赖项 (CentOS)
sudo yum install bzip2 bzip2-devel bzip2-libs python-devel gcc-c++

# 下载 Boost 1.70.0 源码，也可以下载更高版本的
mkdir -p ~/dev/compile/boost & cd ~/dev/compile/boost
wget https://boostorg.jfrog.io/artifactory/main/release/1.70.0/source/boost_1_70_0.tar.gz
tar -xzf boost_1_70_0.tar.gz
cd boost_1_70_0

# 编译安装
./bootstrap.sh --prefix=/usr/local
./b2 -j6   # -j指定编译线程数
sudo ./b2 install
sudo ldconfig

# 卸载
# sudo rm -rf /usr/local/lib/libboost_*
# sudo rm -rf /usr/local/include/boost
```
>

### 2.1 `Linux`

```shell
# 方式1：使用xmake构建
xmake f -m debug     # 可选debug、reelase、releasedbg
xmake b http_server  # 编译静态库
xmake b example      # 编译示例程序
xmake b example2     # 编译示例程序2
# xmake project -k makefile   # 生成makefile文件

# 方式2：使用`make`构建
make -j
```

编译生成的静态库路径：`lib/linux/debug/libhttp_server.a`

```shell
# 运行示例程序：
bin/example
bin/example2
```

浏览器地址栏输入 [`http://127.0.0.1:8099/`](http://127.0.0.1:8099/) 访问。关于示例程序，请参考说明文档 [`example/README.md`](example/README.md)


## 2.2 `Windows`

使用 `Visual Studio 2019` 软件打开`http_server.sln`工程文件。

项目属性页面，修改本机`boost`库的头文件路径，然后正常编译即可。


## 3. 简单使用

```cpp
// main.cpp
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
    router->AddStaticRoute("/server/stop", HttpMethod::kGET, [](Request& req, Json::Value& res){
        req.svr()->Stop();
        res["code"] = 0;
        res["msg"] = "OK";
        res["data"]["time"] = time(NULL);
    });

    // 4. 启动服务器
    if (svr.Listen()) {
        svr.Start();
    }

    return 0;
}
```

## 4. 控制器模式

```cpp
// user_controller.h
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
```

该头文件，经过脚本`route_collector/route_collector.py`解析后，即可生成路由注册的代码。(可批量处理控制器目录下的所有头文件)

```cpp
// Generated by script route_collector.py
#include <server/router.h>
#include "controller/user/user_controller.h"

bool register_routes(std::shared_ptr<ic::server::Router> router) {
    using namespace ic::server;
    bool ret = true;

    // controller/user/user_controller.h
    ret &= router->AddStaticRoute("/api/User/GetInfo", HttpMethod::kGET | HttpMethod::kPOST, controller::UserController::GetInfo, "获取用户信息.", {{"Authorization", "1"}});
    ret &= router->AddStaticRoute("/api/User/Login", HttpMethod::kPOST, controller::UserController::Login, "登录.", {{"Authorization", "0"}});
    ret &= router->AddStaticRoute("/api/User/UploadAvatar", HttpMethod::kPOST, controller::UserController::UploadAvatar, "上传用户头像.", {{"Authorization", "1"}});
    ret &= router->AddRegexRoute("/api/User/avatar/([a-z0-9]{2})/([a-z0-9]{32}\\.(jpg|jpeg|png|gif))", HttpMethod::kGET, controller::UserController::GetAvatarImage, "获取头像，返回二进制图片文件.", {{"Authorization", "1"}});

    return ret;
}
```

`Visual Studio`和`Visual Studio Code`中，可以通过定义`Snippets`，快速生成`Doxygen格式的注释`。

```json
// VS Code
// Ctrl+Shift+P  -->  Snippets: Configure User Snippets  -->  cpp.json
{
    "HttpServer controller comment": {
        "prefix": "/-",
        "body": [
            "/**",
            " * @brief $1.",
            " *",
            " * @route  /$2",
            " * @method GET",
            " */"
        ],
        "description": "Insert comment for HttpServer Controller"
    }
}
```

详情请参考文档 [`route_collector/README.md`](route_collector/README.md)，以及 [`example/README.md`](example/README.md)。

## 5. 检查并获取请求参数(URL参数、Body参数等)

可以使用如下几个方法获取参数

```cpp
// URL参数
// 例如：/xxx?value1=large&value2=small
bool exist;
const std::string& value1 = req.GetUrlParam("value1", &exist);

// Path参数(正则表达式路由)
// 例如路由：/user/([a-zA-Z0-9_]+)/([0-9]+)
std::string name = req.GetRouteRegexMatch(0);     // ([a-zA-Z0-9_]+) 匹配用户名
int page = std::atoi(req.GetRouteRegexMatch(1));  // ([0-9]+) 匹配页码

// Body参数(application/x-www-form-urlencoded)
bool exist;
const std::string& value1 = req.GetBodyParam("value1", &exist);

// Body参数(application/json)
const Json::Value& value1 = req.GetJsonParam("value1");
// value1.isNull() 判断参数是否存在

// Body参数(multipart/form-data)
const FormItem* value1 = req.GetFormItem("value1");
// 如果参数不存在则value1为空指针
value1->is_file()
value1->filename();
value1->content_type();
value1->content();
```

另外，通过以下3个头文件中定义的宏，可以轻松实现参数检查和取值。

+ [`<server/helper/helper.h>`](include/server/helper/helper.h)
+ [`<server/helper/param_check.h>`](include/server/helper/param_check.h)
+ [`<server/helper/param_get.h>`](include/server/helper/param_get.h)

例如某用户登录接口，需要传递`uid`, `password`两个字符串参数和一个`captcha`整形参数，如果登录成功，返回`token`，否则返回`登录失败`信息。

```cpp
/**
 * @brief 登录.
 * 
 * @route  /api/User/Login
 * @method POST
 * @config Authorization(Skip)
 */
void Login(Request& req, Response& res) {
    API_INIT();   // from <server/helper/helper.h>，固定写法，放在函数开头

    // 下面这一行代码，将定义两个 std::string 类型的变量，uid 和 password
    // 检查请求的参数中是否有 uid 和 password，如果不存在将返回“参数缺失”的错误信息
    // 如果存在，则将参数值赋值给 uid 和 password 变量。
    CHECK_JSON_PARAM_STR(uid, password);  // from <server/helper/param_check.h>
    // 同上，这里定义的是 int32_t 类型的变量 captcha
    CHECK_JSON_PARAM_INT(captcha);

    // 检验用户名和密码是否正确 (仅供参考)
    std::string token;
    bool success = CheckLogin(uid, password, captcha, &token);
    if (success) {
        data["token"] = token;   // data的类型是Json::Value，在第一行 API_INIT() 宏函数中定义
        RETURN_OK();
    }
    else {
        RETURN_CODE_MSG(status::base::kWrongUserOrPassword, "用户名或密码错误");
    }
}
```

如果登录成功，将返回如下格式的响应:

```json
{
    "code": 0,
    "msg": "OK",
    "data": {
        "token": "xxxxxxx...."
    }
}
```

## 6. 其他

关于请求拦截器、响应拦截器、路由管理、服务器配置等功能，请参考 `example` 示例程序代码。

## END

> GitHub: [Leopard-C/HttpServer](https://github.com/Leopard-C/HttpServer)
>
> Email: <leopard.c@outlook.com>
