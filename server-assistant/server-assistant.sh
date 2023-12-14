#!/bin/bash

# 获取脚本文件所在路径
script_abs=$(readlink -f "$0")
script_dir=$(dirname $script_abs)

# 根据实际情况修改下面3个变量值
controller_dir=$script_dir/../src/controller
output_file=$script_dir/../src/routes.cpp
include_prefix=controller

# 调用python脚本 server-assistant.py
python3 "$script_dir/server-assistant.py" parse-controller "$controller_dir" "$output_file" -p $include_prefix -y
