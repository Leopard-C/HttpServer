add_rules("mode.debug", "mode.release", "mode.releasedbg")
set_policy("build.warning", true)
set_languages("c99", "cxx11")

add_cxflags("-Wreturn-type", "-Wsign-compare", "-Wunused-variable", "-Wswitch")
add_cxflags("-Wno-deprecated-declarations")

-- arm64编译工具链
toolchain("aarch64")
    set_kind("standalone")
    set_toolset("cc", "aarch64-linux-gnu-gcc")
    set_toolset("cxx", "aarch64-linux-gnu-g++")
    set_toolset("ar", "aarch64-linux-gnu-ar")
    set_toolset("ld", "aarch64-linux-gnu-g++")
toolchain_end()

if is_arch("aarch64") then
    set_toolchains("aarch64")
end

-- boost库依赖配置
add_includedirs("/usr/local/boost/boost_1.73.0/$(arch)/include")
add_linkdirs("/usr/local/boost/boost_1.73.0/$(arch)/lib")

-- 第三方库(submodule)依赖配置
add_includedirs("third_party/jsoncpp/include")
add_includedirs("third_party/spdlog-wrapper/spdlog/include")
add_includedirs("third_party/spdlog-wrapper/spdlog-wrapper/include")
add_linkdirs("third_party/jsoncpp/lib/$(plat)/$(arch)")
add_linkdirs("third_party/spdlog-wrapper/spdlog-wrapper/lib/$(plat)/$(arch)")

-- 全局配置
add_includedirs("include")
set_objectdir("build/obj/$(plat)/$(arch)/$(mode)")

--
-- http_server静态库
--
target("http_server")
    set_kind("static")
    add_files("src/server/**.cpp")
    set_targetdir("lib/$(plat)/$(arch)/$(mode)")

--
-- 示例程序
--
target("example")
    set_kind("binary")
    add_files("example/**.cpp")
    add_includedirs("example/src")
    add_deps("http_server")
    add_links("spdlog_wrapper", "spdlog", "jsoncpp", "boost_regex", "pthread", "dl")
    add_linkorders("http_server", "spdlog_wrapper", "spdlog", "jsoncpp", "pthread", "dl")
    set_targetdir("bin")

--
-- 示例程序2
--
target("example2")
    set_kind("binary")
    add_files("example2/**.cpp")
    add_deps("http_server")
    add_links("spdlog_wrapper", "spdlog", "jsoncpp", "boost_regex", "pthread", "dl")
    add_linkorders("http_server", "spdlog_wrapper", "spdlog", "jsoncpp", "pthread", "dl")
    set_targetdir("bin")
