#pragma once
#include "globals.hpp"
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL2_rotozoom.h>
#include <SDL2/SDL_surface.h>
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

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"




class Clabel{
    private:
    Cordinates cords{0,0};
    SDL_Renderer* renderer=nullptr;
    bool drawBackground=false,absoluteCordinates=false,visible=true;
    SDL_Color textColor{255,255,255,255};
    TTF_Font* font=nullptr;
    SDL_Texture* texture=nullptr;
    int Nexty;
    int labelH=0;
    
    enum class IconPosition {
    LEFT,
    RIGHT
    };
    IconPosition iconPosition = IconPosition::LEFT;
    int iconWidth = 0;
    int iconHeight = 0;
    int spacing = 5;


    SDL_Texture* LoadSVG(SDL_Renderer* renderer, const char* filename, float scale)
        {
            
            NSVGimage* image = nsvgParseFromFile(filename, "px", 96.0f);
            if (!image) {
                std::cout << "Failed to load SVG\n";
                return nullptr;
            }

            int width = image->width * scale;
            int height = image->height * scale;

            
            NSVGrasterizer* rast = nsvgCreateRasterizer();

            
            unsigned char* img = new unsigned char[width * height * 4];

           
            nsvgRasterize(rast, image, 0, 0, scale, img, width, height, width * 4);

            
            SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
                img,
                width,
                height,
                32,
                width * 4,
                0x000000ff,
                0x0000ff00,
                0x00ff0000,
                0xff000000
            );

            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

            SDL_FreeSurface(surface);
            delete[] img;
            nsvgDeleteRasterizer(rast);
            nsvgDelete(image);

            return texture;
        }

    public:

    Clabel(){




    }
    Clabel(SDL_Renderer* r,Cordinates c,bool db, bool abs,TTF_Font * f){

        renderer=r;
        cords=c;
        drawBackground=db;
        absoluteCordinates=abs;
        font=f;





    }

    Clabel(SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){

        renderer=r;
        cords=c;
        drawBackground=db;
        absoluteCordinates=abs;
        font=f;
        visible=v;
        textColor=tc;




    }
    ~Clabel(){
        if(texture) SDL_DestroyTexture(texture);
        std::cout<<"Destroy Label "<<std::endl;
    }

    void RenderText(const std::string& text)
    {
        if (!visible) return;
        if (!font || !renderer) return;

        SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), textColor);
        if (!textSurface) return;

        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (!textTexture)
        {
            SDL_FreeSurface(textSurface);
            return;
        }

        int textW = textSurface->w;
        int textH = textSurface->h;

        SDL_FreeSurface(textSurface);

        // Base Y position
        int baseY = absoluteCordinates ? cords.y : cords.y - textH;

        // Calculate total width if icon exists
        int totalWidth = textW;
        if (texture)
            totalWidth += iconWidth + spacing;

        int currentX = cords.x;
         labelH=textH;
        // -------- Background --------
        if (drawBackground)
        {
            SDL_Rect bgRect;
            bgRect.x = cords.x - 5;
            bgRect.y = baseY - 5;
            bgRect.w = totalWidth + 10;
            bgRect.h = std::max(textH, iconHeight) + 10;
            labelH=bgRect.h;

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 76, 76, 76, 150);
            SDL_RenderFillRect(renderer, &bgRect);
        }

        // -------- Render LEFT icon --------
        if (texture && iconPosition == IconPosition::LEFT)
        {
            SDL_Rect iconRect;
            iconRect.x = currentX;
            iconRect.y = baseY + (textH - iconHeight) / 2;
            iconRect.w = iconWidth;
            iconRect.h = iconHeight;
            //labelH=iconHeight;
            SDL_RenderCopy(renderer, texture, nullptr, &iconRect);

            currentX += iconWidth + spacing;
        }

        // -------- Render Text --------
        SDL_Rect textRect;
        textRect.x = currentX;
        textRect.y = baseY;
        textRect.w = textW;
        textRect.h = textH;

        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

        currentX += textW + spacing;

        // -------- Render RIGHT icon --------
        if (texture && iconPosition == IconPosition::RIGHT)
        {
            SDL_Rect iconRect;
            iconRect.x = currentX;
            iconRect.y = baseY + (textH - iconHeight) / 2;
            iconRect.w = iconWidth;
            iconRect.h = iconHeight;

            SDL_RenderCopy(renderer, texture, nullptr, &iconRect);
        }

        Nexty = baseY + std::max(textH, iconHeight) + 10;

        SDL_DestroyTexture(textTexture);
    }

void Render(Cordinates c,const std::string& text){
    if(!visible) return;
    cords.x=c.x;
    cords.y=c.y;
    RenderText(text);
    //std::cout<<"Render label "<<cords.x<<" "<<text<<std::endl;




}

void Render(const std::string& text){
    if(!visible) return;
    RenderText(text);
    //std::cout<<"Render label "<<cords.x<<" "<<text<<std::endl;




}

void LoadSVGtoLabel(const char* filename, float scale = 1.0f)
{
    if (texture)
        SDL_DestroyTexture(texture);

    texture = LoadSVG(renderer, filename, scale);

    if (texture)
    {
        SDL_QueryTexture(texture, nullptr, nullptr, &iconWidth, &iconHeight);
    }
}




int getNexty(){return Nexty;}
int isVisible(){return visible;}
int getLabelH(){return  labelH;}


void setVisibility(bool b){visible=b;}
void setCords(int x,int y){


    cords.x=x;
    cords.y=y;
}

void setIconPositionLeft()  { iconPosition = IconPosition::LEFT; }
void setIconPositionRight() { iconPosition = IconPosition::RIGHT; }







};





class CDebugLabels{

    private:
    std::vector<std::unique_ptr<Clabel>> labels;
    SDL_Renderer *renderer=nullptr;
    Cordinates cords;
    bool visible=true;
    TTF_Font* font=nullptr;


    void CreateLabels(int n){

        for(int i=0; i<n;i++)
        labels.push_back(std::make_unique<Clabel>(renderer, Cordinates{cords.x,cords.y}, true, true,false ,font,SDL_Color{255,0,0,255}));
    }

    public:

    CDebugLabels(SDL_Renderer * r,Cordinates c,TTF_Font* f){

        renderer=r;
        cords=c;
        font=f;




    }




    void Render(const std::vector<std::string>& strs){
        if(strs.size()<0) return;

        if(strs.size()>labels.size())
            CreateLabels(strs.size()-labels.size());


       
        labels[0]->setCords(cords.x, cords.y);
        labels[0]->RenderText(strs[0]);

        for(int i=1; i<strs.size();i++){

            labels[i]->Render({cords.x,labels[i-1]->getNexty()},strs[i]);


        }
        

        






    }
    void setCords(int x,int y){


    cords.x=x;
    cords.y=y;
    }

    void setVisibility(bool b){

        for(int i=0;i<labels.size();i++){


            labels[i]->setVisibility(b);
        }
    }








};