@echo off
setlocal

REM 获取脚本文件所在路径
for %%i in ("%~dp0.") do set "script_dir=%%~fi"

REM 根据实际情况修改下面3个变量值
set controller_dir=%script_dir%/../src/controller
set output_file=%script_dir%/../src/routes.cpp
set include_prefix=controller

REM 调用python脚本 server-assistant.py
python "%script_dir%\server-assistant.py" parse-controller "%controller_dir%" "%output_file%" -p %include_prefix% -y

endlocal