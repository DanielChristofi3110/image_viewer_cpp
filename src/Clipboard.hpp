#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif

class CClipboard {
public:

    void copyImageToClipboard(SDL_Surface* surface)
    {
        if (!surface) return;

#ifdef _WIN32
        copyToWindowsClipboard(surface);
#else
        copyToWaylandClipboard(surface);
#endif
    }

private:

#ifdef _WIN32
    void copyToWindowsClipboard(SDL_Surface* surf)
    {
        if (!surf) return;

        SDL_Surface* converted = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
        if (!converted) return;

        int width = converted->w;
        int height = converted->h;
        int bytesPerPixel = 4;
        int imageSize = width * height * bytesPerPixel;

        BITMAPINFOHEADER bi = {};
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = width;
        bi.biHeight = -height; // negative for top-down bitmap
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = imageSize;

        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + imageSize);
        if (!hMem) {
            SDL_FreeSurface(converted);
            return;
        }

        void* ptr = GlobalLock(hMem);
        memcpy(ptr, &bi, sizeof(BITMAPINFOHEADER));
        memcpy((char*)ptr + sizeof(BITMAPINFOHEADER), converted->pixels, imageSize);
        GlobalUnlock(hMem);

        if (OpenClipboard(NULL)) {
            EmptyClipboard();
            SetClipboardData(CF_DIB, hMem);
            CloseClipboard();
            std::cout << "Copied image to Windows clipboard\n";
        } else {
            std::cerr << "Failed to open Windows clipboard\n";
            GlobalFree(hMem);
        }

        SDL_FreeSurface(converted);
    }
#else
    void copyToWaylandClipboard(SDL_Surface* surf)
    {
        if (!surf) return;

        SDL_Surface* converted =
            SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);

        if (!converted) return;

        std::vector<unsigned char> pngData;
        auto writeFunc = [](void* context, void* data, int size) {
            auto* vec = (std::vector<unsigned char>*)context;
            unsigned char* bytes = (unsigned char*)data;
            vec->insert(vec->end(), bytes, bytes + size);
        };

        int result = stbi_write_png_to_func(
            writeFunc,
            &pngData,
            converted->w,
            converted->h,
            4,
            converted->pixels,
            converted->pitch
        );

        SDL_FreeSurface(converted);

        if (!result) {
            std::cerr << "Failed to encode PNG for clipboard\n";
            return;
        }

        // Pipe PNG to wl-copy
        FILE* pipe = popen("wl-copy --type image/png", "w");
        if (!pipe) {
            std::cerr << "Failed to open wl-copy pipe\n";
            return;
        }

        fwrite(pngData.data(), 1, pngData.size(), pipe);
        pclose(pipe);

        std::cout << "Copied image to Wayland clipboard (PNG)\n";
    }
#endif
};