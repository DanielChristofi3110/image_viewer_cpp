#!/bin/bash
g++ src/main.cpp -o build/viewer -std=c++17 -I/usr/include/SDL2 -D_REENTRANT -lSDL2_image -lSDL2_ttf -lSDL2