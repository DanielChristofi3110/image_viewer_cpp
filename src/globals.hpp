#pragma once
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



#define DEBUG true
#define FULL_PRELOAD false
#define ASYNCLOADING 10
#define UNLOADAT 10


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





SDL_Texture* CreateRadialGradientTexture(SDL_Renderer* renderer, int width, int height,SDL_Color color)
{
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    );

    if (!texture)
        return nullptr;

    void* pixels;
    int pitch;

    if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) != 0)
        return nullptr;

    Uint32* pixelBuffer = (Uint32*)pixels;

    SDL_Color centerColor = color;
    SDL_Color edgeColor   = {0, 0, 0, 255};   // black edges

    float centerX = width / 2.0f;
    float centerY = height / 2.0f;
    float maxDist = std::sqrt(centerX * centerX + centerY * centerY);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float dx = x - centerX;
            float dy = y - centerY;

            float dist = std::sqrt(dx * dx + dy * dy);
            float t = dist / maxDist;
            t = std::clamp(t, 0.0f, 1.0f);

            // Optional smooth curve
            t = pow(t, 1.3f);

            Uint8 r = (Uint8)(centerColor.r + t * (edgeColor.r - centerColor.r));
            Uint8 g = (Uint8)(centerColor.g + t * (edgeColor.g - centerColor.g));
            Uint8 b = (Uint8)(centerColor.b + t * (edgeColor.b - centerColor.b));

           SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);

            Uint32 color = SDL_MapRGBA(format, r, g, b, 255);

            SDL_FreeFormat(format);
            pixelBuffer[y * (pitch / 4) + x] = color;
        }
    }

    SDL_UnlockTexture(texture);

    return texture;
}



