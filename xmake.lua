add_rules("mode.debug", "mode.release", "mode.releasedbg")
set_policy("build.warning", true)
set_languages("c99", "cxx11")

add_cxflags("-Wreturn-type", "-Wsign-compare", "-Wunused-variable", "-Wswitch")
add_cxflags("-Wno-deprecated-declarations")

mode = "debug"
if is_mode("debug") then
    mode = "debug"
elseif is_mode("release") then
    mode = "release"
elseif is_mode("releasedbg") then
    mode = "releasedbg"
end

add_includedirs("third_party/jsoncpp/include")
add_includedirs("third_party/spdlog-wrapper/spdlog/include")
add_includedirs("third_party/spdlog-wrapper/spdlog-wrapper/include")
add_linkdirs(string.format("lib/linux/%s", mode), "lib/linux")
add_linkdirs("third_party/jsoncpp/lib/linux")
add_linkdirs("third_party/spdlog-wrapper/spdlog-wrapper/lib/linux")
set_objectdir(string.format("build/obj/%s", mode))

target("http_server")
    set_kind("static")
    add_files("src/server/**.cpp")
    add_includedirs("include/server")
    set_targetdir(string.format("lib/linux/%s", mode))

target("example")
    set_kind("binary")
    add_files("example/**.cpp")
    add_includedirs("include", "example/src")
    add_deps("http_server")
    add_links("spdlog_wrapper", "spdlog", "jsoncpp", "boost_regex", "pthread", "dl")
    add_linkorders("http_server", "spdlog_wrapper", "spdlog", "jsoncpp", "pthread", "dl")
    set_targetdir("bin")

target("example2")
    set_kind("binary")
    add_files("example2/**.cpp")
    add_includedirs("include")
    add_deps("http_server")
    add_links("spdlog_wrapper", "spdlog", "jsoncpp", "boost_regex", "pthread", "dl")
    add_linkorders("http_server", "spdlog_wrapper", "spdlog", "jsoncpp", "pthread", "dl")
    set_targetdir("bin")

