#pragma once
#include "globals.hpp"
#include "GUI.hpp"
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL2_rotozoom.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_pixels.h>

#include <SDL2/SDL_ttf.h>
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>



// A stroke = one continuous drag
struct Stroke {
    SDL_Color color;
    std::vector<Cordinates> points;
};

class CCanvas {

private:
    SDL_Renderer* renderer;
    SDL_Color Current_color{255, 0, 0, 255};

    std::vector<Stroke> strokes;   // all finished strokes
    Stroke currentStroke;          // stroke being drawn
    bool drawing = false;
    std::vector<std::unique_ptr<Clabel>> PenIcons;
    const char * SvgIconPath=nullptr; 
    uint64_t currentPen=0;

    SDL_Point RotateAroundCenter(int x, int y, int cx, int cy, int rotation) {
            // move to center
            int tx = x - cx;
            int ty = y - cy;

            SDL_Point r = RotatePoint(tx, ty, rotation);

            // move back
            return { r.x + cx, r.y + cy };
        }
    SDL_Point RotatePoint(int x, int y, int rotation) {

            switch (rotation) {
                case 90:  return { -y,  x };
                case 180: return { -x, -y };
                case 270: return {  y, -x };
                default:  return {  x,  y }; // 0 or 360
            }
        }

    SDL_Color InvertColor(const SDL_Color& color){
            SDL_Color inverted;
            inverted.r = 255 - color.r;
            inverted.g = 255 - color.g;
            inverted.b = 255 - color.b;
            inverted.a = color.a; // keep alpha unchanged
            return inverted;
        }
    int IsColorDark(const SDL_Color& color)
    {

        float luminance = 0.299f * color.r + 0.587f * color.g + 0.114f * color.b;


        if (luminance < 128.0f)
            return 0; 
        else
            return 1;
    }
public:

    CCanvas(SDL_Renderer* r,TTF_Font *f,const char * Path) {
        renderer = r;
        SvgIconPath=Path;

        //PenIcons.reserve(2);
        // Clabel(const std::string txt, SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){
        PenIcons.push_back((std::make_unique<Clabel>("",r,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255})));
        PenIcons[0]->LoadSVGtoLabel(SvgIconPath,{255,255,255,255},0.03f);
        PenIcons[0]->setBackgroundColor({0,0,0,0});


        PenIcons.push_back((std::make_unique<Clabel>("",r,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255})));
        PenIcons[1]->LoadSVGtoLabel(SvgIconPath,{0,0,0,255},0.03f);
        PenIcons[1]->setBackgroundColor({0,0,0,0});
    }

    // Call when mouse button is pressed
    void StartStroke(int x, int y) {
        drawing = true;
        currentStroke = Stroke{};
        currentStroke.color = Current_color;
        currentStroke.points.push_back({x, y});
    }

    // Call while dragging
    void DrawOn(int mouseX, int mouseY) {
        if (!drawing) return;

        currentStroke.points.push_back({mouseX, mouseY});
    }

    // Call when mouse button is released
    void EndStroke() {
        if (drawing && !currentStroke.points.empty()) {
            strokes.push_back(currentStroke);
        }
        drawing = false;
    }

    // Undo last stroke
    void Undo() {
        if (!strokes.empty()) {
            strokes.pop_back();
        }
    }

    // Render everything
void Render(int offX, int offY, float zoom, int rotation) {

    // Draw all saved strokes
    for (const auto& stroke : strokes) {
        SDL_SetRenderDrawColor(renderer,
                               stroke.color.r,
                               stroke.color.g,
                               stroke.color.b,
                               stroke.color.a);

        for (size_t i = 1; i < stroke.points.size(); i++) {

            SDL_Point p1 = RotatePoint(stroke.points[i - 1].x,
                                       stroke.points[i - 1].y,
                                       rotation);

            SDL_Point p2 = RotatePoint(stroke.points[i].x,
                                       stroke.points[i].y,
                                       rotation);

            int x1 = static_cast<int>(p1.x * zoom + offX);
            int y1 = static_cast<int>(p1.y * zoom + offY);
            int x2 = static_cast<int>(p2.x * zoom + offX);
            int y2 = static_cast<int>(p2.y * zoom + offY);

            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }

    // Draw current stroke
    if (drawing) {
        SDL_SetRenderDrawColor(renderer,
                               currentStroke.color.r,
                               currentStroke.color.g,
                               currentStroke.color.b,
                               currentStroke.color.a);

        for (size_t i = 1; i < currentStroke.points.size(); i++) {

            SDL_Point p1 = RotatePoint(currentStroke.points[i - 1].x,
                                       currentStroke.points[i - 1].y,
                                       rotation);

            SDL_Point p2 = RotatePoint(currentStroke.points[i].x,
                                       currentStroke.points[i].y,
                                       rotation);

            int x1 = static_cast<int>(p1.x * zoom + offX);
            int y1 = static_cast<int>(p1.y * zoom + offY);
            int x2 = static_cast<int>(p2.x * zoom + offX);
            int y2 = static_cast<int>(p2.y * zoom + offY);

            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }
}
    void RenderPen(int mx,int my){

        PenIcons[currentPen]->Render(Cordinates{mx,my-PenIcons[currentPen]->getLabelH()/2+1});
    }

    // Optional: change color
    void SetColor(SDL_Color color) {
        Current_color = color;
    }

    void clear(){

        strokes.clear();
        currentStroke.points.clear();
        drawing = false;
    }

    bool isDrawing()const {return drawing;}

    void setPenInvertedColor(SDL_Color c){

       // PenIcon->LoadSVGtoLabel(SvgIconPath,InvertColor(c),0.03f);
       currentPen=IsColorDark(c);
    }
};