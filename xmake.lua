add_rules("mode.debug", "mode.release", "mode.releasedbg")
set_policy("build.warning", true)
set_languages("c99", "cxx11")

add_cxflags("-Wreturn-type", "-Wsign-compare", "-Wunused-variable", "-Wswitch", "-Werror")
add_cxflags("-Wno-unused-result", "-Wno-deprecated-declarations", "-Wno-unused-parameter")

-- boost库依赖配置
add_includedirs("/usr/local/boost/boost_1.73.0/$(arch)/include")
add_linkdirs("/usr/local/boost/boost_1.73.0/$(arch)/lib")

-- 全局配置
add_includedirs("include")
set_objectdir("build/obj/$(plat)/$(arch)/$(mode)")

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
    add_links("boost_regex", "pthread", "dl")
    add_linkorders("http_server", "boost_regex", "pthread", "dl")
    set_targetdir("bin")

--
-- 示例程序2
--
target("example2")
    set_kind("binary")
    add_files("example2/**.cpp")
    add_deps("http_server")
    add_links("boost_regex", "pthread", "dl")
    add_linkorders("http_server", "boost_regex", "pthread", "dl")
    set_targetdir("bin")
