
--
-- 检查某个路径类型的配置项(例如头文件路径、库文件路径)是否存在且有效
--
function check_option_dir(option_name)
    local dir = get_config(option_name)
    if dir and dir ~= "" then
        if not os.isdir(dir) then
            cprint("${red}error:${clear} The dir '" .. dir .. "' specified in option '" .. option_name .. "' does not exist.")
            return -1
        end
        return 0
    else
        return 1
    end
end

--
-- 检查`boost`库配置
--
function _check_boost_config()
    local ret_boost_include = check_option_dir("boost_include_dir")
    local ret_boost_lib = check_option_dir("boost_lib_dir")
    if ret_boost_include == -1 or ret_boost_lib == -1 then
        return false
    elseif ret_boost_include == 0 and ret_boost_lib == 0 then
        return true
    else
        local ret_boost_root = check_option_dir("boost_root_dir")
        return ret_boost_root == 0
    end
end

--
-- 检查`boost`库配置
--
function check_boost_config()
    if not _check_boost_config() then
        print_usage_boost_config()
        os.raise('stop')
    end
end

--
-- 打印`boost`库配置方法
--
function print_usage_boost_config()
    cprint("${red}error:${clear} Please configure boost dir first! (e.g.)")
    cprint("${red}error:${clear}    xmake f --mode=release --boost_root_dir=/usr/local/boost/1.73.0");
    cprint("${red}error:${clear} or")
    cprint("${red}error:${clear}    xmake f --mode=release --boost_include_dir=/usr/local/boost/1.73.0/include --boost_lib_dir=/usr/local/boost/1.73.0/lib");
end
