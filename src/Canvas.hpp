#pragma once
#include "globals.hpp"
#include "GUI.hpp"
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL2_rotozoom.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL2_gfxPrimitives.h> // Make sure SDL_gfx is included

#include <SDL2/SDL_ttf.h>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
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
    uint16_t thickness=10;
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
    bool reset_flag=true;

    const uint64_t Pallet_size=3;
    SDL_Color ColorPallet[3] ={
        {255,0,0,255},
        {0,255,0,255},
        {0,0,255,255}
    
    };



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
    int leastColorIndex(const SDL_Color& color) {
        if (color.r <= color.g && color.r <= color.b) {
            return 0; // Red is least present
        } else if (color.g <= color.r && color.g <= color.b) {
            return 1; // Green is least present
        } else {
            return 2; // Blue is least present
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
        // PenIcons.push_back((std::make_unique<Clabel>("",r,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255})));
        // //172, 171, 196
        // PenIcons[0]->LoadSVGtoLabel(SvgIconPath,{172,171,196,255},0.03f);
        // PenIcons[0]->setBackgroundColor({0,0,0,0});


        // PenIcons.push_back((std::make_unique<Clabel>("",r,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255})));
        // //7, 4, 89
        // PenIcons[1]->LoadSVGtoLabel(SvgIconPath,{7,4,89,255},0.03f);
        // PenIcons[1]->setBackgroundColor({0,0,0,0});

        for(uint64_t i=0; i<Pallet_size; i++){
            // Clabel(const std::string txt, SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){
        PenIcons.push_back((std::make_unique<Clabel>("",r,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255})));
        //172, 171, 196
        PenIcons.back()->LoadSVGtoLabel(SvgIconPath,ColorPallet[i],0.03f);
        PenIcons.back()->setBackgroundColor({0,0,0,0});


        }
    }

    // Call when mouse button is pressed
    void StartStroke(int x, int y,uint16_t thi) {
        drawing = true;
        currentStroke = Stroke{};
        currentStroke.color = Current_color;
        currentStroke.points.push_back({x, y});
       currentStroke.thickness=thi;
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

        // Uint8 r = stroke.color.r;
        // Uint8 g = stroke.color.g;
        // Uint8 b = stroke.color.b;
        // Uint8 a = stroke.color.a;

        Uint32 gfxColor = (stroke.color.a << 24) | 
                  (stroke.color.b << 16) |
                  (stroke.color.g << 8) |
                  (stroke.color.r);

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

            if (stroke.thickness <= 1) {
                // Anti-aliased line if thickness is 1
                aalineColor(renderer, x1, y1, x2, y2, gfxColor);
            } else {
                // Thick line for thickness > 1
                thickLineColor(renderer, x1, y1, x2, y2, stroke.thickness, gfxColor);
            }
        }
    }

    // Draw current stroke
    if (drawing) {
       
         Uint32 gfxColor = (currentStroke.color.a << 24) | 
                  (currentStroke.color.b << 16) |
                  (currentStroke.color.g << 8) |
                  (currentStroke.color.r);

        
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

            if (currentStroke.thickness <= 1) {
                aalineColor(renderer, x1, y1, x2, y2, gfxColor);
            } else {
                thickLineColor(renderer, x1, y1, x2, y2, currentStroke.thickness, gfxColor);
            }
        }


    }
}
    void RenderPen(int mx,int my,int th){

        PenIcons[currentPen]->Render(Cordinates{mx,my-PenIcons[currentPen]->getLabelH()/2-5});

          Uint32 gfxColor = (Current_color.a << 24) | 
                  (Current_color.b << 16) |
                  (Current_color.g << 8) |
                  (Current_color.r);
        aacircleRGBA(renderer, mx, my, th/2, Current_color.r,Current_color.g,Current_color.b,Current_color.a);
    }

    // Optional: change color
    void SetColor(SDL_Color color) {
        Current_color = color;
    }

    void clear(){

        strokes.clear();
        currentStroke.points.clear();
        drawing = false;
        reset_flag=true;
    }

    bool isDrawing()const {return drawing;}

    void setPenInvertedColor(SDL_Color c){

        if(!reset_flag) return;
       // PenIcon->LoadSVGtoLabel(SvgIconPath,InvertColor(c),0.03f);
       currentPen=leastColorIndex(c);
       Current_color=ColorPallet[currentPen];
       

    }

    void nextColor(){
        
        currentPen++;
        currentPen=currentPen%Pallet_size;
         std::cout<<"Next color "<<currentPen<<std::endl;
        Current_color=ColorPallet[currentPen];
        reset_flag=false;
    }
    SDL_Color getCurrentPenColor(){return Current_color;}
};