--
-- 该配置文件，仅适用`Linux`操作系统。在`Windows`平台下请使用`VS2019+`打开`http_server.sln`工程文件。
--
add_rules("mode.debug", "mode.release", "mode.releasedbg")
set_policy("build.warning", true)
set_languages("c99", "cxx11")

add_cxflags("-Wreturn-type", "-Wsign-compare", "-Wunused-variable", "-Wswitch", "-Werror")
add_cxflags("-Wno-unused-result", "-Wno-deprecated-declarations", "-Wno-unused-parameter")

-- `boost`头文件和库文件目录配置
boost_inc_dir = "/usr/local/boost/boost_1.73.0/x86_64/include"
boost_lib_dir = "/usr/local/boost/boost_1.73.0/x86_64/lib"

add_includedirs(boost_inc_dir, "include")
add_linkdirs(boost_lib_dir)

--
-- http_server静态库
--
target("http_server")
    set_kind("static")
    add_files("src/server/**.cpp", "src/jsoncpp/*.cpp")
    set_targetdir("lib/$(plat)/$(arch)/$(mode)")

--
-- 示例程序
--
target("example")
    set_kind("binary")
    add_files("example/**.cpp")
    add_includedirs("example/src")
    add_deps("http_server")
    add_links("boost_regex", "boost_thread", "pthread", "dl")
    add_linkorders("http_server", "boost_regex", "boost_thread", "pthread", "dl")
    set_targetdir("bin")

--
-- 示例程序2
--
target("example2")
    set_kind("binary")
    add_files("example2/**.cpp")
    add_deps("http_server")
    add_links("boost_regex", "boost_thread", "pthread", "dl")
    add_linkorders("http_server", "boost_regex", "boost_thread", "pthread", "dl")
    set_targetdir("bin")
