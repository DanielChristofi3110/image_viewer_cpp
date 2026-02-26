#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_ttf.h>

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



#define DEBUG true
#define FULL_PRELOAD false

const int THUMB_WIDTH = 100;
const int THUMB_HEIGHT = 75;
const int INIT_THUMB_X = 10;
const int INIT_THUMB_Y = 10;
const int THUMB_PADDING = 10;

namespace fs = std::filesystem;


struct Cordinates{int x,y;};

bool free_mode=false;
bool debug_mode=false;
bool Loadthumbnails=true;
int fps=0;


//image loading
 SDL_Texture* loadImage(const std::string& path, SDL_Renderer* renderer, int& w, int& h) {
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        std::cout << "Failed to load: " << path << "\n";
        return nullptr;
    }
    w = surf->w;
    h = surf->h;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
    }