@echo off

rd /s /q build 
mkdir build
cd build

setlocal

set QT_DIR=D:\dev\install\Qt\6.5.3\msvc2019_64
set CMAKE_DIR=D:\dev\install\Qt\Tools\CMake_64\bin
set PATH=%QT_DIR%\bin;%CMAKE_DIR%;%PATH%

:: 使用 CMake 配置项目
cmake -G "Visual Studio 16 2019" ..

:: 生成
cmake --build . --config Release

endlocal