@echo off
echo ===== Flash Writer by SkyJay =====
set /p port=�����봮�ںţ����� COM5��:
set /p path=������ .bin �ļ�·��:
set /p baud=�����벨���ʣ�Ĭ��115200��:

if "%baud%"=="" (
    set baud=115200
)

WriteToFlash.exe %port% %path% %baud%

pause
