## 一个简单的网页

+ 登录功能：用户名密码认证成功后，后台接口将token写入cookie中(HttpOnly)，之后的接口请求会自动携带token。
+ 用户信息编辑功能：登录完成后，进入用户信息编辑页面。
+ 上传头像：通过内容类型为`multipart/form-data`的`POST`请求上传头像图片到服务器。

## 访问方式

启动服务器后，在浏览器地址栏输入：[http://127.0.0.1:8099](http://127.0.0.1:8099)，即可访问。

## 用户认证

+ 用户名 `admin`
+ 密码 `123456`

生成的token有效期10分钟，10分钟后，token失效，页面跳转至登录页面。

示例程序中的token颁发、认证方式比较简单，如需要更完善的token认证机制，可以选择 `JWT (Json Web Token)`。

用户数据保存在 `data/system/user.dat` 文件中。

## 目录结构

```shell
src
├── app             # 应用程序
│   ├── app.cpp
│   └── app.h
├── controller     # 控制器
│   ├── server
│   │   ├── server_controller.cpp
│   │   └── server_controller.h
│   ├── staticfile
│   │   ├── staticfile_controller.cpp
│   │   └── staticfile_controller.h
│   ├── test
│   │   ├── test_controller.cpp
│   │   └── test_controller.h
│   ├── user
│   │   ├── user_controller.cpp
│   │   └── user_controller.h
│   └── web
│       ├── web_controller.cpp
│       └── web_controller.h
├── main.cpp
├── manager
│   ├── user_manager.cpp
│   └── user_manager.h
├── routes.cpp   # 路由注册，由脚本 route_collector.py 生成
├── routes.h
└── singleton
    └── singleton.h
```

```shell
# 通过脚本，生成注册控制器中所有路由的代码
cd example
script/route_collector.py src/controller src/routes.cpp -p controller
# 或者 (Linux)
script/route_collector.sh
# 或者 (Windows)
script\route_collector.bat
```

## 接口列表

+ ` GET` /
+ ` GET` /index
+ ` GET` /index.html
+ ` GET` /favicon.ico
+ ` GET` /staticfile/(.+)
+ ` GET` /web/(.+)
+ ` GET` /api/Test/TestEmpty
+ `POST` /api/User/Login
+ ` GET` /api/User/GetInfo
+ `POST` /api/User/UpdateInfo
+ `POST` /api/User/UploadAvatar
+ `POST` /api/Server/Shutdown
+ `POST` /api/Server/DumpThreadInfos

## 接口测试

使用 Apifox软件 导入 [apifox/http_server_example.apifox.json](apifox/http_server_example.apifox.json) 项目文件。

