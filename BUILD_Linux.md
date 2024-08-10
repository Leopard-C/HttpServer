# 编译 HttpServer 静态库及示例程序 (Linux)

+ Linux
+ [Windows](./BUILD_Windows.md)

## 1 编译安装`Boost` (如果已安装请跳过)

```shell
# 安装依赖项 (Ubuntu)
sudo apt install build-essential g++ autotools-dev libicu-dev build-essential python-dev libbz2-dev
# 安装依赖项 (CentOS)
sudo yum install bzip2 bzip2-devel bzip2-libs python-devel gcc-c++

# 下载 Boost 1.73.0 源码，也可以下载更高版本的
mkdir -p ~/dev/compile/boost && cd ~/dev/compile/boost
wget https://boostorg.jfrog.io/artifactory/main/release/1.73.0/source/boost_1_73_0.tar.gz
tar -xzf boost_1_73_0.tar.gz && cd boost_1_73_0

# 编译静态库
./bootstrap.sh
./b2 stage -a -j6 cxxflags='-fPIC -std=c++11 -O3' --without-python --stagedir="stage" link=static

# 安装静态库
sudo mkdir -p /usr/local/boost/boost_1.73.0/x86_64/include
sudo cp -r boost /usr/local/boost/boost_1.73.0/x86_64/include
sudo cp -r stage/lib /usr/local/boost/boost_1.73.0/x86_64
```

## 2 编译`HttpServer`静态库及示例程序

> 本项目在`Linux`平台通过`xmake`工具进行构建。
> 
> 安装`xmake`方式:
> 
> ```shell
> wget https://xmake.io/shget.text -O - | bash
> ```
> 
> 详情请参考官方仓库 [xmake-io/xmake](https://github.com/xmake-io/xmake) 或文档: [xmake.io](https://xmake.io/#/zh-cn/guide/installation)

首先修改该项目的配置文件`xmake.lua`，找到如下内容，进行修改

```lua
-- `boost`头文件和库文件目录配置
boost_inc_dir = "/usr/local/boost/boost_1.73.0/x86_64/include"
boost_lib_dir = "/usr/local/boost/boost_1.73.0/x86_64/lib"
```

然后执行构建命令：

```shell
# 设置debug或release模式
# xmake f -m debug
# xmake f -m release

# 静态库
xmake b http_server

# 示例程序1
xmake b example

# 示例程序2
xmake b example2

# 或一次性构建所有工程
xmake
```

运行示例程序：

```shell
bin/example
```

浏览器地址栏输入 [`http://127.0.0.1:8099/`](http://127.0.0.1:8099/) 访问。关于示例程序，请参考说明文档 [`example/README.md`](example/README.md)

## END
