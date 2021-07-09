@echo off

call vcvars64
set QTDIR=c:\qt6\6.1.2\msvc2019_64
set PATH=c:\qt6\tools\cmake_64\bin;%QTDIR%\bin;%PATH%
set CMAKE_PREFIX_PATH=%QTDIR%

if not exist target goto init
cd target
exit /b

:init
mkdir target
cd target
cmake -G "NMake Makefiles" ..

