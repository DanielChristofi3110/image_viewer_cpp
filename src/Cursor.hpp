#pragma once
#include "globals.hpp"
#include "image.hpp"
// #include "GUI.hpp"
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>


class CCursor {

private:

    SDL_Cursor* cursorArrow;
    SDL_Cursor* cursorIBeam;
    SDL_Cursor* cursorWait;
    SDL_Cursor* cursorCrosshair;
    SDL_Cursor* cursorWaitArrow;
    SDL_Cursor* cursorSizeNWSE;
    SDL_Cursor* cursorSizeNESW;
    SDL_Cursor* cursorSizeWE;
    SDL_Cursor* cursorSizeNS;
    SDL_Cursor* cursorSizeAll;
    SDL_Cursor* cursorNo;
    SDL_Cursor* cursorHand;

public:

    enum cursorType {
        Arrow,
        IBeam,
        Wait,
        Crosshair,
        WaitArrow,
        SizeNWSE,
        SizeNESW,
        SizeWE,
        SizeNS,
        SizeAll,
        No,
        Hand,
        Hide
    };

    cursorType currentCursor = Arrow;

    CCursor()
    {
        cursorArrow     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
        cursorIBeam     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
        cursorWait      = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
        cursorCrosshair = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
        cursorWaitArrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
        cursorSizeNWSE  = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
        cursorSizeNESW  = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
        cursorSizeWE    = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
        cursorSizeNS    = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
        cursorSizeAll   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
        cursorNo        = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
        cursorHand      = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    }

    ~CCursor()
    {
        SDL_FreeCursor(cursorArrow);
        SDL_FreeCursor(cursorIBeam);
        SDL_FreeCursor(cursorWait);
        SDL_FreeCursor(cursorCrosshair);
        SDL_FreeCursor(cursorWaitArrow);
        SDL_FreeCursor(cursorSizeNWSE);
        SDL_FreeCursor(cursorSizeNESW);
        SDL_FreeCursor(cursorSizeWE);
        SDL_FreeCursor(cursorSizeNS);
        SDL_FreeCursor(cursorSizeAll);
        SDL_FreeCursor(cursorNo);
        SDL_FreeCursor(cursorHand);

        std::cout<<"Cursor Destroyed"<<std::endl;
    }

    void setCursor(cursorType ct)
    {
        if (ct == currentCursor) return;
        SDL_ShowCursor(SDL_ENABLE);
        currentCursor = ct;

        switch (ct)
        {
            case Arrow:     SDL_SetCursor(cursorArrow); break;
            case IBeam:     SDL_SetCursor(cursorIBeam); break;
            case Wait:      SDL_SetCursor(cursorWait); break;
            case Crosshair: SDL_SetCursor(cursorCrosshair); break;
            case WaitArrow: SDL_SetCursor(cursorWaitArrow); break;
            case SizeNWSE:  SDL_SetCursor(cursorSizeNWSE); break;
            case SizeNESW:  SDL_SetCursor(cursorSizeNESW); break;
            case SizeWE:    SDL_SetCursor(cursorSizeWE); break;
            case SizeNS:    SDL_SetCursor(cursorSizeNS); break;
            case SizeAll:   SDL_SetCursor(cursorSizeAll); break;
            case No:        SDL_SetCursor(cursorNo); break;
            case Hand:      SDL_SetCursor(cursorHand); break;
            case Hide:      SDL_ShowCursor(SDL_DISABLE);    break;
        }
    }
};