#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <stdio.h>
#include "globals.hpp"
#ifdef _WIN32
#include <windows.h>
#endif
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


class CClipboard {
public:

    void copyImageToClipboard(SDL_Surface* surface)
    {
        if (!surface) return;

#ifdef _WIN32
        copyToWindowsClipboard(surface);
#elif __linux__

    bool hasWlCopy = commandExists("wl-copy");
    bool hasXclip  = commandExists("xclip");

    if (getenv("WAYLAND_DISPLAY") && hasWlCopy)
    {
        copyToWaylandClipboard(surface);
    }
    else if (hasXclip)
    {
        copyToX11Clipboard(surface);
    }
    else
    {
        std::cerr << "No clipboard tool found (wl-copy or xclip)\n";
    }

#endif
    }

private:

#ifdef __linux__
bool commandExists(const char* cmd)
{
    std::string check = "command -v ";
    check += cmd;
    check += " >/dev/null 2>&1";

    int result = system(check.c_str());
    return result == 0;
}

#endif

#ifdef _WIN32
void copyToWindowsClipboard(SDL_Surface* surf)
{
    if (!surf) return;

    SDL_Surface* converted =
        SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);

    if (!converted) return;

    int width = converted->w;
    int height = converted->h;
    int imageSize = converted->pitch * height;

    // ---------- Encode PNG ----------
    std::vector<unsigned char> pngData;

    auto writeFunc = [](void* context, void* data, int size)
    {
        auto* vec = (std::vector<unsigned char>*)context;
        unsigned char* bytes = (unsigned char*)data;
        vec->insert(vec->end(), bytes, bytes + size);
    };

    stbi_write_png_to_func(
        writeFunc,
        &pngData,
        width,
        height,
        4,
        converted->pixels,
        converted->pitch
    );

    UINT pngFormat = RegisterClipboardFormatA("PNG");

    HGLOBAL hMemPNG = GlobalAlloc(GMEM_MOVEABLE, pngData.size());
    void* ptrPNG = GlobalLock(hMemPNG);
    memcpy(ptrPNG, pngData.data(), pngData.size());
    GlobalUnlock(hMemPNG);

    // ---------- Create DIBV5 ----------
    BITMAPV5HEADER bi = {};
    bi.bV5Size = sizeof(BITMAPV5HEADER);
    bi.bV5Width = width;
    bi.bV5Height = -height;
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;

    bi.bV5RedMask   = 0x00FF0000;
    bi.bV5GreenMask = 0x0000FF00;
    bi.bV5BlueMask  = 0x000000FF;
    bi.bV5AlphaMask = 0xFF000000;

    HGLOBAL hMemV5 = GlobalAlloc(
        GMEM_MOVEABLE,
        sizeof(BITMAPV5HEADER) + imageSize
    );

    BYTE* ptrV5 = (BYTE*)GlobalLock(hMemV5);

    memcpy(ptrV5, &bi, sizeof(BITMAPV5HEADER));
    memcpy(ptrV5 + sizeof(BITMAPV5HEADER),
           converted->pixels,
           imageSize);

    GlobalUnlock(hMemV5);

    // ---------- Classic DIB ----------
    BITMAPINFOHEADER bih = {};
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = -height;
    bih.biPlanes = 1;
    bih.biBitCount = 32;
    bih.biCompression = BI_RGB;

    HGLOBAL hMemDIB = GlobalAlloc(
        GMEM_MOVEABLE,
        sizeof(BITMAPINFOHEADER) + imageSize
    );

    BYTE* ptrDIB = (BYTE*)GlobalLock(hMemDIB);

    memcpy(ptrDIB, &bih, sizeof(BITMAPINFOHEADER));
    memcpy(ptrDIB + sizeof(BITMAPINFOHEADER),
           converted->pixels,
           imageSize);

    GlobalUnlock(hMemDIB);

    // ---------- Bitmap ----------
    HDC hdc = GetDC(NULL);

    BITMAPINFO bmi = {};
    bmi.bmiHeader = bih;

    void* dibPixels = nullptr;

    HBITMAP hBitmap = CreateDIBSection(
        hdc,
        &bmi,
        DIB_RGB_COLORS,
        &dibPixels,
        NULL,
        0
    );

    memcpy(dibPixels, converted->pixels, imageSize);

    ReleaseDC(NULL, hdc);

    // ---------- Clipboard ----------
    if (OpenClipboard(NULL))
    {
        EmptyClipboard();

        SetClipboardData(pngFormat, hMemPNG); // Discord / Chromium
        SetClipboardData(CF_DIBV5, hMemV5);   // Modern apps
        SetClipboardData(CF_DIB, hMemDIB);    // Office fallback
        SetClipboardData(CF_BITMAP, hBitmap); // GDI apps

        CloseClipboard();

       if(DEBUG)  std::cout << "Copied image to clipboard (PNG + DIB + BITMAP)\n";
    }
    else
    {
        GlobalFree(hMemPNG);
        GlobalFree(hMemV5);
        GlobalFree(hMemDIB);
        DeleteObject(hBitmap);
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

        if(DEBUG) std::cout << "Copied image to Wayland clipboard (PNG)\n";
    }


    void copyToX11Clipboard(SDL_Surface* surf)
    {
        if (!surf) return;

        SDL_Surface* converted =
            SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);

        if (!converted) return;

        std::vector<unsigned char> pngData;

        auto writeFunc = [](void* context, void* data, int size)
        {
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

        if (!result)
        {
            std::cerr << "Failed to encode PNG\n";
            return;
        }

        // Pipe to xclip
        FILE* pipe = popen(
            "xclip -selection clipboard -t image/png -i",
            "w"
        );

        if (!pipe)
        {
            std::cerr << "Failed to run xclip\n";
            return;
        }

        fwrite(pngData.data(), 1, pngData.size(), pipe);
        pclose(pipe);

        if(DEBUG) std::cout << "Copied image to X11 clipboard (PNG)\n";
    }
#endif
};