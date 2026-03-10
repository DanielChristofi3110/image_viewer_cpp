@echo off
rem This batch script will compile the image viewer program in debug mode

windres app.rc -O coff -o app.res
rem Set the source and build directories
set SRC_DIR=src
set BUILD_DIR=build

rem Set the include and lib directories for MinGW and SDL2
set MINGW_INCLUDE_DIR=C:\mingw64\include
set MINGW_LIB_DIR=C:\mingw64\lib

rem Set the compiler and flags
set COMPILER=g++
set FLAGS=-g -O0 -std=c++17 app.res -D_REENTRANT -g -O0 
rem -g enables debug symbols, -O0 disables optimization for easier debugging

set INCLUDE_FLAGS=-I"%MINGW_INCLUDE_DIR%\SDL2" -I"%MINGW_INCLUDE_DIR%\SDL2_image" -I"%MINGW_INCLUDE_DIR%\SDL2_ttf"
set LIB_FLAGS=-L"%MINGW_LIB_DIR%" -lmingw32 -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_gfx -ldwmapi

rem Compile the program
%COMPILER% %SRC_DIR%\main.cpp -o %BUILD_DIR%\viewer_debug.exe %FLAGS% %INCLUDE_FLAGS% %LIB_FLAGS%

rem Check if the compile was successful
if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed.
) else (
    echo Debug compilation succeeded. Executable created at %BUILD_DIR%\viewer_debug.exe
)

pause
