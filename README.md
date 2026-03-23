# ImageViewer

A simple cross-platform image viewer built with SDL2 and related libraries.
This project demonstrates a minimal setup for loading and displaying images using SDL2 with CMake.

## Features

-  **Load and display common image formats (PNG, JPG, etc.)** -
-  **SDL2-based rendering**
-  **Cross-platform support (Windows & Linux)** 
-  **Simple packaging support via CPack (DEB, RPM, TGZ)t**
## Requirements
###   General
-  **CMake ≥ 3.16**
-  **C++17 compatible compiler**
A detailed description of what this project does and who it's for. Explain the problem it solves and why it's useful.

### Linux

Install dependencies using your package manager:

#### Ubuntu / Debian:

```bash
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev
```

### Windows (MSVC)
-  **Visual Studio (recommended)**
Prebuilt SDL2 libraries placed in:

```bash
libs/
├── include
├── lib
├── bin
```
Required libraries:

-  **SDL2**
-  **SDL2_image**
-  **SDL2_ttf**
-  **SDL2_gfx**

## Build Instructions
### Linux

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Run:
```bash
./viewer
```
### Windows (Visual Studio)
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```
Executable will be in:

build/Release/viewer.exe

DLLs will be copied automatically after build.


