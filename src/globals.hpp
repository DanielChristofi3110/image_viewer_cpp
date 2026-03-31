#pragma once
// #include "GUI.hpp"
// #include "Cursor.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_rotozoom.h>
// sudo pacman -S sdl2_gfx
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <iterator>
#include <ostream>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>

namespace fs = std::filesystem;

#ifdef _WIN32
#define FIX_WINDOWS true
#define DEBUG false

#ifdef _DEBUG
    #define DEBUG true
#endif

#elif __unix__
#define FIX_WINDOWS false
#ifdef NDEBUG
    #define DEBUG false //no debug
#else
    #define DEBUG true
#endif

#endif





#define FULL_PRELOAD false

#define THUMBNAIL_ASYNCLOADING true //leave true

const int THUMB_WIDTH = 100;
const int THUMB_HEIGHT = 75;
const int INIT_THUMB_X = 10;
const int INIT_THUMB_Y = 10;
const int THUMB_PADDING = 10;



struct Cordinates{int x,y;};

bool free_mode=true;
bool debug_mode=false;
bool Loadthumbnails=true;
int fps=0;
std::string execDir=".";
std::string resDir=".";
std::string confDir=".";

int ASYNCLOADING= 1;
int UNLOADAT= 2;
int MAXIMAGE_QUEUE=10;





#ifdef _WIN32
std::string delim="\\";
#elif __unix__
std::string delim="/";
#endif




