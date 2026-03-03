#pragma once
#include "globals.hpp"
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>






class CBackground {

private:
    SDL_Color cColor{0,0,0,255};  
    SDL_Color startColor;
    SDL_Color targetColor;

    SDL_Renderer* renderer;

    float lerpDuration = 1.0f;     
    float lerpProgress = 0.0f;    
    bool isLerping = false;

    Uint8 LerpChannel(Uint8 a, Uint8 b, float t) {
        return static_cast<Uint8>(a + (b - a) * t);
    }

public:

    CBackground(SDL_Renderer* r) {
        renderer = r;
    }

  
    void StartLerp(SDL_Color newColor, float duration) {
        if(isLerping) return;
        startColor = cColor;
        targetColor = newColor;
        lerpDuration = duration;
        lerpProgress = 0.0f;
        isLerping = true;
    }

  
    void Update(float deltaTime) {
        if (!isLerping)
            return;

        lerpProgress += deltaTime / lerpDuration;

        if (lerpProgress >= 1.0f) {
            lerpProgress = 1.0f;
            isLerping = false;
        }

        cColor.r = LerpChannel(startColor.r, targetColor.r, lerpProgress);
        cColor.g = LerpChannel(startColor.g, targetColor.g, lerpProgress);
        cColor.b = LerpChannel(startColor.b, targetColor.b, lerpProgress);
        cColor.a = LerpChannel(startColor.a, targetColor.a, lerpProgress);
    }

    void Render() {
        SDL_SetRenderDrawColor(renderer, cColor.r, cColor.g, cColor.b, cColor.a);
        SDL_RenderClear(renderer);
    }
};