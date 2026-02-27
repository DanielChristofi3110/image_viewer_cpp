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
SDL_Colour gAvgColor;



SDL_Color GetAverageColor(SDL_Surface* surface)
{
    Uint64 totalR = 0;
    Uint64 totalG = 0;
    Uint64 totalB = 0;

    int pixelCount = surface->w * surface->h;

    SDL_LockSurface(surface);

    Uint8* pixels = (Uint8*)surface->pixels;
    int bpp = surface->format->BytesPerPixel;

    for (int y = 0; y < surface->h; y++)
    {
        for (int x = 0; x < surface->w; x++)
        {
            Uint8* p = pixels + y * surface->pitch + x * bpp;

            Uint32 pixelValue;
            memcpy(&pixelValue, p, bpp);

            Uint8 r, g, b;
            SDL_GetRGB(pixelValue, surface->format, &r, &g, &b);

            totalR += r;
            totalG += g;
            totalB += b;
        }
    }

    SDL_UnlockSurface(surface);

    SDL_Color avg;
    avg.r = totalR / pixelCount;
    avg.g = totalG / pixelCount;
    avg.b = totalB / pixelCount;
    avg.a = 255;

    return avg;
}
//image loading
 SDL_Texture* loadImage(const std::string& path, SDL_Renderer* renderer, int& w, int& h) {
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        std::cout << "Failed to load: " << path << "\n";
        return nullptr;
    }
    w = surf->w;
    h = surf->h;

    //std::cout << "Avg color " << int(avgColor.r)<<" "<<int(avgColor.g)<<" "<<int(avgColor.b) << std::endl;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
    }


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

