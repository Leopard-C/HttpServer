# Http Server

<p align="center">
  <img alt="C++11" src="https://img.shields.io/badge/Language-C++11-brightgreen" />
  <img alt="Boost" src="https://img.shields.io/badge/Boost->=1.73.0-brightgreen" />
  <img alt="Windows& Linux" src="https://img.shields.io/badge/platform-windows%20%7C%20linux-brightgreen" />
  <img alt="MIT-License" src="https://img.shields.io/badge/License-MIT-blue" />
  <a href="https://github.com/Leopard-C/HttpServer/wiki"><img src="https://img.shields.io/badge/Documentation-Wiki-brightgreen" /></a>
</p>

基于 `Boost.Beast` 实现的跨平台、高性能的HTTP服务器。

## 1. 文档

+ 项目Wiki: [Http Server Wiki](https://github.com/Leopard-C/HttpServer/wiki)
+ 构建文档：
    + Windows平台：[BUILD_Windows.md](BUILD_Windows.md)
    + Linux平台：[BUILD_Linux.md](BUILD_Linux.md)

## 2. 依赖

+ C++11
+ Boost 1.73.0
+ [Boost.Asio](https://github.com/boostorg/asio), [Boost.Beast](https://github.com/boostorg/beast), [Boost.Regex](https://github.com/boostorg/regex)（可选，性能比 std::regex 性能高些）
+ [Leopard-C/jsoncpp](https://github.com/Leopard-C/jsoncpp) （修改自[open-source-parsers/jsoncpp](https://github.com/open-source-parsers/jsoncpp)）
+ [gabime/spdlog](https://github.com/gabime/spdlog)
+ [Leopard-C/spdlog-wrapper](https://github.com/Leopard-C/spdlog-wrapper)


## 3. 功能要点

+ 支持`Windows`、`Linux`平台
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


## 4. 简单使用

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

## 5. 控制器模式

```cpp
// user_controller.h
#pragma once
#include <server/request.h>
#include <server/response.h>

using Request = ic::server::Request;
using Response = ic::server::Response;

class UserController {
public:
    /**
     * @brief 登录.
     * @route  /api/User/Login
     * @method POST
     * @config Authorization(0)
     */
    static void Login(Request& req, Response& res);

    /**
     * @brief 上传用户头像.
     * @route  /api/User/UploadAvatar
     * @method POST
     * @config Authorization(1)
     */
    static void UploadAvatar(Request& req, Response& res);

    /**
     * @brief 获取头像，返回二进制图片文件.
     * @route  ~ /api/User/avatar/([a-z0-9]{2})/([a-z0-9]{32}\.(jpg|jpeg|png|gif))
     * @method GET
     * @config Authorization(1)
     */
    static void GetAvatarImage(Request& req, Response& res);
};
```

该头文件，经过脚本`server-assistant/server-assistant.py`解析后，即可生成路由注册的代码。(可批量处理控制器目录下的所有头文件)

```cpp
// Generated by script server-assistant.py
#include <server/router.h>
#include "controller/user/user_controller.h"

bool register_routes(std::shared_ptr<ic::server::Router> router) {
    using namespace ic::server;
    bool ret = true;

    // controller/user/user_controller.h
    ret &= router->AddStaticRoute("/api/User/Login", HttpMethod::kPOST, UserController::Login, "登录.", {{"Authorization", "0"}});
    ret &= router->AddStaticRoute("/api/User/UploadAvatar", HttpMethod::kPOST, UserController::UploadAvatar, "上传用户头像.", {{"Authorization", "1"}});
    ret &= router->AddRegexRoute("/api/User/avatar/([a-z0-9]{2})/([a-z0-9]{32}\\.(jpg|jpeg|png|gif))", HttpMethod::kGET, UserController::GetAvatarImage, "获取头像，返回二进制图片文件.", {{"Authorization", "1"}});

    return ret;
}
```

详情请参考脚本说明文档 [`server-assistant/README.md`](server-assistant/README.md)，以及 [`example/README.md`](example/README.md)。

## 6. 检查并获取请求参数(URL参数、Body参数等)

主要有三种方式：

+ 【基础】通过`Request`类的`GetUrlParam()`, `GetBodyParam()`, `GetJsonParam()`方法获取参数值，并手动进行类型转换。
+ 【推荐】通过宏`GET_URL_PARAM_INT(param1, param2, ...)`, `GET_BODY_PARAM_xxx(...)`获取参数值，会自动进行类型转换
    + 支持`std::string`, `bool`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`, `double`类型。
+ 【推荐】通过`DTO (Data Transfer Object)`方式获取参数值，只需在结构体或类中引入引入宏`DTO_IN`或`DTO_OUT`，即可通过脚本自动生成序列化、反序列化代码。

### 6.1 方式一：获取参数值，并手动类型转换

该库的`ic::server::Request`类提供如下方法，用于获取请求参数。

最为灵活，运行效率最高，但是需要手动进行类型检查和转换，代码量可能较大。

```cpp
// URL参数
// 例如：/xxx?value1=large&value2=small
bool exist;
const std::string& value1 = req.GetUrlParam("value1", &exist);

// Path参数(正则表达式路由)
// 例如路由：/user/([a-zA-Z0-9_]+)/([0-9]+)
const std::string& name = req.GetRouteRegexMatch(0);     // ([a-zA-Z0-9_]+) 匹配用户名
int page = std::atoi(req.GetRouteRegexMatch(1).c_str()); // ([0-9]+) 匹配页码

// Body参数(application/x-www-form-urlencoded)
// 通过 req.content_type().IsApplicationXWwwFormUrlEncoded() 判断是否该内容类型
bool exist;
const std::string& value1 = req.GetBodyParam("value1", &exist);

// Body参数(application/json)
// 通过 req.content_type().IsApplicationJson() 判断该是否该内容类型
const Json::Value& value1 = req.GetJsonParam("value1");
// value1.isNull() 判断参数是否存在

// Body参数(multipart/form-data)
// 通过 req.content_type().IsMultipartFormData() 判断该是否该内容类型
const FormItem* value1 = req.GetFormItem("value1");
// 如果参数不存在则value1为空指针
value1->is_file()
value1->filename();
value1->content_type();
value1->content();
```

### 6.2 方式二：使用宏 `CHECK_PARAM_xxx` 和 `GET_PARAM_xxx`

通过以下3个头文件中定义的宏，可以轻松实现参数检查和取值。

+ [`<server/helper/helper.h>`](include/server/helper/helper.h)
+ [`<server/helper/param_check.h>`](include/server/helper/param_check.h)
+ [`<server/helper/param_get.h>`](include/server/helper/param_get.h)

```cpp
// 检查URL中是否有名称为 param1, param2, ..., paramN 的参数 (一条语句最多检查10个参数)
// 如果没有，则直接响应 `参数缺失` 的错误
// 如果有，但不是int类型，则直接响应 `参数无效` 的错误
// 如果有，且为int类型，将定义 int 类型的变量 param1, param2, ... paramN，并赋予相应的参数值
CHECK_URL_PARAM_INT(param1, param2, ..., paramN);    // int32_t
CHECK_URL_PARAM_UINT(param1, param2, ..., paramN);   // uint32_t
CHECK_URL_PARAM_INT64(param1, param2, ..., paramN);  // int64_t
CHECK_URL_PARAM_UINT64(param1, param2, ..., paramN); // uint64_t
CHECK_URL_PARAM_BOOL(param1, param2, ..., paramN);   // bool
CHECK_URL_PARAM_DOUBLE(param1, param2, ..., paramN); // double
CHECK_URL_PARAM_STR(param1, param2, ..., paramN);    // std::string

// 检查URL中是否有名称为 param1, param2, ..., paramN 的参数
// 如果没有，则使用指定的默认值
// 如果有，但不是int类型，则直接响应 `参数无效` 的错误
// 如果有，且为int类型，将定义 int 类型的变量 param1, param2, ... paramN，并赋予相应的参数值。
GET_URL_PARAM_INT(default_value, param1, param2, ..., paramN);    // int32_t
GET_URL_PARAM_xxx(default_value, param1, param2, ..., paramN);

// 相应的，还有从body参数中取值的宏
CHECK_BODY_PARAM_xxx()
GET_BODY_PARAM_xxx()

CHECK_JSON_PARAM_xxx()
GET_JSON_PARAM_xxx()

CHECK_BODY_PARAM_xxx_EX()
GET_BODY_PARAM_xxx_EX()
```

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

### 6.3 方式三：通过`DTO (Data Transfer Object)`方式获取参数值

包含头文件`<server/helper/dto.h>`

通过如下方式定义`DTO`结构体或类。

```cpp
struct LoginDto {
    DOT_IN;   // 必选 DTO_IN, DTO_OUT, DTO_IN_OUT 中的一种

    /**
     * @brief 账号ID.
     * @required
     */
    std::string uid;

    /**
     * @brief 密码(md5哈希值).
     * @alias password
     * @required
     */
    std::string password_md5;

    /**
     * @brief 用户角色.
     * @ignore
     */
    std::string role;

    /**
     * @brief 时间戳.
     * @optional
     */
    time_t timestamp = 0;
};
```

需要借助 `server-assistant/server-assistant.py` 脚本，批量处理所有`DTO`头文件，生成序列化、反序列化方法的实现代码。

+ `DTO_IN`: 用于接收请求数据，将生成 `bool Deserialize(const Json::Value&)` 反序列化方法。
+ `DTO_OUT`: 作为接口响应数据，将生成 `Json::Value Serialize() const` 序列化方法。
+ `DTO_IN_OUT`: 以上两个的融合。

接口参数扩展属性：

+ `@required`: 该参数必选。（默认行为）
+ `@optional`: 该参数可选，在定义的地方指定默认值。
+ `@ignore`: 序列化、反序列化时忽略该参数。
+ `@alias {alias}`: 指定在`JSON`对象中的别名。

支持的成员变量类型：

+ 基础数据类型`(u)int32_t`, `(u)int64_t`, `bool`, `float`, `double`, `std::string`。
+ 其他`DTO`类型，如 `RoleDto`
+ STL容器类型，如`vector<int>`, `set<uint64_t>`, `vector<RoleDto>`
    + STL容器：`vector`, `deque`, `list`, `set`, `unordered_set`
    + 元素类型：`基础数据类型` 或 `其他DTO`

## 7. 其他

关于请求拦截器、响应拦截器、路由管理、服务器配置等功能，请参考 `example` 示例程序代码、项目Wiki: [Http Server Wiki](https://github.com/Leopard-C/HttpServer/wiki) 。

## END

> GitHub: [Leopard-C/HttpServer](https://github.com/Leopard-C/HttpServer)
>
> Email: <leopard.c@outlook.com>
