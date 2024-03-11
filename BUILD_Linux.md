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

可以通过`make`或`xmake`两种方式进行编译。

### 2.1 `make`

```shell
make -j
```

### 2.2 `xmake`【推荐】

修改`xmake.lua`中`boost`库的配置。

```lua
-- xmake.lua
-- boost库依赖配置
add_includedirs("/usr/local/boost/boost_1.73.0/$(arch)/include")
add_linkdirs("/usr/local/boost/boost_1.73.0/$(arch)/lib")
```

然后执行如下命令：

```shell
xmake f --mode=debug     # 可选debug、reelase、releasedbg
xmake
# xmake project -k makefile   # 生成makefile文件
```

运行示例程序：

```shell
bin/example
```

浏览器地址栏输入 [`http://127.0.0.1:8099/`](http://127.0.0.1:8099/) 访问。关于示例程序，请参考说明文档 [`example/README.md`](example/README.md)

---

## 3 交叉编译

支持交叉编译，例如在`x86_64`机器上编译`aarch64`架构机器中可执行程序。首先需要交叉编译依赖库(boost)，然后执行如下命令进行编译：

```shell
xmake f --mode=release --arch=aarch64    # 默认--arch=x86_64
xmake
```

## END
