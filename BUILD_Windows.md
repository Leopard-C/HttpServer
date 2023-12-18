# 编译 HttpServer 静态库及示例程序 (Windows)

+ [Linux](./BUILD_Linux.md)
+ Windows

## 1 编译安装`Boost` (如果已安装请跳过)

下载boost 1.73.0版本源码：https://boostorg.jfrog.io/artifactory/main/release/1.73.0/source/boost_1_73_0.tar.gz

解压后进入`boost_1.73_0`目录，打开`命令行`，执行如下命令：

```shell
# 生成构建程序
bootstrap.bat

# 编译boost静态库, 一次性编译x86|x64, debug|release
b2 stage -a -j6 --toolset=msvc-14.2 --address-model=32,64 --architecture=x86 --without-test --without-python --stagedir="stage" link=static runtime-link=shared threading=multi debug release

# 编译完成后，将 boost 和 stage/lib 复制到指定位置
# 创建安装目录
md D:\CPP_INCLUDE\boost_1.73.0\include\boost
md D:\CPP_INCLUDE\boost_1.73.0\lib
# 复制头文件
xcopy /E /I /Y boost D:\CPP_INCLUDE\boost_1.73.0\include\boost
# 复制静态库
xcopy /E /I /Y stage\lib D:\CPP_INCLUDE\boost_1.73.0\lib
```

+ `stage`: 只生成库(lib)
+ `-a`: 重新构建所有内容。
+ `-j6`: 指定编译并行数量
+ `--toolset=msvc-14.2`: 指定编译器(VS2019)
    + `14.0` 1900 (VS2015 14.0)
    + `14.1` 1910 (VS2017 15.0)
    + `14.2` 1920 (VS2019 16.0)
    + `14.3` 1930 (VS2022 17.0)
+ `--address-model=32,64`: 同时编译`32`和`64`位的库
+ `--architecture=x86`: CPU架构(x86)
+ `--without-test`: 不构建`test`模块
+ `--without-python`: 不构建`python`模块
+ `--stagedir=stage`: 生成的`lib`库文件路径
+ `link=static`: 生成静态库`.lib`，不生成`.dll`
+ `runtime-link=shared`: 动态链接C++运行时库

## 2 编译子模块

```shell
# 克隆项目，并拉取子模块
git clone https://github.com/Leopard-C/HttpServer
git submodule update --init --recursive
```

+ `third_party/jsoncpp`
+ `third_party/spdlog-wrapper`

按照子模块的 `README.md` 文档中的方法，编译两个子模块。


## 3 编译`HttpServer`静态库及示例程序

使用`Visual Studio 2019`软件打开`http_server.sln`项目文件，修改`boost`库头文件路径和库文件路径。

然后正常编译即可。

编译完成后，运行示例程序：

```shell
bin\example_x64_Debug.exe
```

浏览器地址栏输入 [`http://127.0.0.1:8099/`](http://127.0.0.1:8099/) 访问。关于示例程序，请参考说明文档 [`example/README.md`](example/README.md)

## END
