@echo off
echo ===== Flash Writer by SkyJay =====
set /p port=请输入串口号（例如 COM5）:
set /p path=请输入 .bin 文件路径:
set /p baud=请输入波特率（默认115200）:

if "%baud%"=="" (
    set baud=115200
)

WriteToFlash.exe %port% %path% %baud%

pause
