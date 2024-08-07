--
-- 该配置文件，仅适用`Linux`操作系统。在`Windows`平台下请使用`VS2019+`打开`http_server.sln`工程文件。
--

add_rules("mode.debug", "mode.release", "mode.releasedbg")
set_policy("build.warning", true)
set_languages("c99", "cxx11")

add_cxflags("-Wreturn-type", "-Wsign-compare", "-Wunused-variable", "-Wswitch", "-Werror")
add_cxflags("-Wno-unused-result", "-Wno-deprecated-declarations", "-Wno-unused-parameter")

-- boost配置
--     (1) xmake f --mode=release --boost_root_dir=/usr/local/boost/1.73.0
--  or (2) xmake f --mode=release --boost_include_dir=/usr/local/boost/1.73.0/include --boost_lib_dir=/usr/local/boost/1.73.0/lib
option("boost_include_dir")
    set_description("The include directory of boost. [Version>=1.73.0]")
    local dir = get_config("boost_include_dir")
    if dir then add_includedirs(dir) end

option("boost_lib_dir")
    set_description("The lib directory of boost. [Version>=1.73.0]")
    local dir = get_config("boost_lib_dir")
    if dir then add_linkdirs(dir) end

option("boost_root_dir")
    set_description("The root directory of boost. [Version>=1.73.0]")
    local dir = get_config("boost_root_dir")
    if dir then
        add_includedirs(dir .. "/include")
        add_linkdirs(dir .. "/lib")
    end

--
-- http_server静态库
--
target("http_server")
    on_load(function()
        import("xmake_util")
        xmake_util.check_boost_config()
    end)
    set_kind("static")
    add_options("boost_root_dir", "boost_include_dir")
    add_includedirs("include")
    add_files("src/server/**.cpp", "src/jsoncpp/*.cpp")
    set_targetdir("lib/$(plat)/$(arch)/$(mode)")

--
-- 示例程序
--
target("example")
    on_load(function()
        import("xmake_util")
        xmake_util.check_boost_config()
    end)
    set_kind("binary")
    add_options("boost_root_dir", "boost_include_dir", "boost_lib_dir")
    add_files("example/**.cpp")
    add_includedirs("include", "example/src")
    add_deps("http_server")
    add_links("boost_regex", "boost_thread", "pthread", "dl")
    add_linkorders("http_server", "boost_regex", "boost_thread", "pthread", "dl")
    set_targetdir("bin")

--
-- 示例程序2
--
target("example2")
    on_load(function()
        import("xmake_util")
        xmake_util.check_boost_config()
    end)
    set_kind("binary")
    add_options("boost_root_dir", "boost_include_dir", "boost_lib_dir")
    add_includedirs("include")
    add_files("example2/**.cpp")
    add_deps("http_server")
    add_links("boost_regex", "boost_thread", "pthread", "dl")
    add_linkorders("http_server", "boost_regex", "boost_thread", "pthread", "dl")
    set_targetdir("bin")
