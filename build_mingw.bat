@echo off
rem This batch script will compile the image viewer program using g++
windres app.rc -O coff -o app.res
rem Set the source and build directories
set SRC_DIR=src
set BUILD_DIR=build

rem Set the include and lib directories for MinGW and SDL2
set MINGW_INCLUDE_DIR=C:\mingw64\include
set MINGW_LIB_DIR=C:\mingw64\lib

rem Set the compiler and flags
set COMPILER=g++
set FLAGS=-std=c++17 -O2 -s  app.res -D_REENTRANT
set INCLUDE_FLAGS=-I"%MINGW_INCLUDE_DIR%\SDL2" -I"%MINGW_INCLUDE_DIR%\SDL2_image" -I"%MINGW_INCLUDE_DIR%\SDL2_ttf"
set LIB_FLAGS=-L"%MINGW_LIB_DIR%" -lmingw32 -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_gfx -mwindows -ldwmapi 



rem Compile the program
%COMPILER% %SRC_DIR%\main.cpp -o %BUILD_DIR%\viewer %FLAGS% %INCLUDE_FLAGS% %LIB_FLAGS%

rem Check if the compile was successful
if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed.
) else (
    echo Compilation succeeded. Executable created at %BUILD_DIR%\viewer.exe
)

pause
