@echo off
echo Copying Qt DLLs...

set QT_PATH=D:\dev\vcpkg\installed\x64-windows
set BUILD_PATH=cmake-build-debug

REM Копіювати основні DLL
copy "%QT_PATH%\bin\Qt6Core.dll" "%BUILD_PATH%\" /Y
copy "%QT_PATH%\bin\Qt6Gui.dll" "%BUILD_PATH%\" /Y
copy "%QT_PATH%\bin\Qt6Widgets.dll" "%BUILD_PATH%\" /Y
copy "%QT_PATH%\bin\Qt6Network.dll" "%BUILD_PATH%\" /Y

REM Створити папку для плагінів
mkdir "%BUILD_PATH%\platforms" 2>nul

REM Копіювати платформні плагіни
copy "%QT_PATH%\plugins\platforms\qwindows.dll" "%BUILD_PATH%\platforms\" /Y
copy "%QT_PATH%\plugins\platforms\qwindowsd.dll" "%BUILD_PATH%\platforms\" /Y

echo Done!
pause