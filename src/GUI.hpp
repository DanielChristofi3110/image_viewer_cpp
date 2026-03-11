#pragma once
#include "globals.hpp"
#include <SDL2/SDL_pixels.h>
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
#include <string>
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
    SDL_Color textColor{255,255,255,255},bg_color{76, 76, 76, 150};
    TTF_Font* font=nullptr;
    SDL_Texture* texture=nullptr;
    int Nexty;
    int labelH=0,labelW=0;
    
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

            //int width = image->width * scale;
            //int height = image->height * scale;
            int width  = std::round(image->width * scale);
            int height = std::round(image->height * scale);

            
            NSVGrasterizer* rast = nsvgCreateRasterizer();

            
            //unsigned char* img = new unsigned char[width * height * 4];
            std::vector<unsigned char> img(width * height * 4);

           
            nsvgRasterize(rast, image, 0, 0, scale, img.data(), width, height, width * 4);

            
            SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
                img.data(),
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

    Clabel(const Clabel&) = delete;
    Clabel& operator=(const Clabel&) = delete;

void RenderText(const std::string& text)
{
    if (!visible) return;
    if (!font || !renderer) return;

    SDL_Surface* textSurface = nullptr;
    SDL_Texture* textTexture = nullptr;

    int textW = 0;
    int textH = 0;

    // -------- Render Text Surface (only if not empty) --------
    if (!text.empty())
    {
        textSurface = TTF_RenderText_Blended(font, text.c_str(), textColor);
        if (textSurface)
        {
            textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            textW = textSurface->w;
            textH = textSurface->h;
            SDL_FreeSurface(textSurface);

            if (!textTexture)
                return;
        }
    }

    // Base Y position
    //int baseY = absoluteCordinates ? cords.y : cords.y - textH;

    // Calculate total width
// Calculate total width
    int totalWidth = textW;

    if (texture)
    {
        if (text.empty())
            totalWidth += iconWidth;
        else
            totalWidth += iconWidth + spacing;
    }

    // Calculate content height FIRST
    int contentHeight = std::max(textH, texture ? iconHeight : 0);

    // Base Y position AFTER height is known
    int baseY = absoluteCordinates ? cords.y : cords.y - contentHeight;

    int currentX = cords.x;

    // -------- Background --------
    if (drawBackground)
    {
        SDL_Rect bgRect;
        bgRect.x = cords.x - 5;
        bgRect.y = baseY - 5;
        bgRect.w = totalWidth + 10;
        bgRect.h = contentHeight + 10;

        labelH = bgRect.h;
        labelW = bgRect.w;

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
        SDL_RenderFillRect(renderer, &bgRect);
    }

    // -------- Render LEFT icon --------
    if (texture && iconPosition == IconPosition::LEFT)
    {
        SDL_Rect iconRect;
        iconRect.x = currentX;
        iconRect.y = baseY + (contentHeight - iconHeight) / 2;
        iconRect.w = iconWidth;
        iconRect.h = iconHeight;

        SDL_RenderCopy(renderer, texture, nullptr, &iconRect);

        currentX += iconWidth + (text.empty() ? 0 : spacing);
    }

    // -------- Render Text --------
    if (textTexture)
    {
        SDL_Rect textRect;
        textRect.x = currentX;
        textRect.y = baseY;
        textRect.w = textW;
        textRect.h = textH;

        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

        currentX += textW + spacing;
    }

    // -------- Render RIGHT icon --------
    if (texture && iconPosition == IconPosition::RIGHT)
    {
        SDL_Rect iconRect;
        iconRect.x = currentX;
        iconRect.y = baseY + (contentHeight - iconHeight) / 2;
        iconRect.w = iconWidth;
        iconRect.h = iconHeight;

        SDL_RenderCopy(renderer, texture, nullptr, &iconRect);
    }

    Nexty = baseY + contentHeight + 10;

    if (textTexture)
        SDL_DestroyTexture(textTexture);
}


void RenderBackground(int width, int height)
{
    if (!visible) return;
    if (!renderer) return;
    if (!drawBackground) return;

    // Base Y position
    int baseY = absoluteCordinates ? cords.y : cords.y - height;

    SDL_Rect bgRect;
    bgRect.x = cords.x;
    bgRect.y = baseY;
    bgRect.w = width;
    bgRect.h = height;

    labelW = width;
    labelH = height;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderFillRect(renderer, &bgRect);

    Nexty = baseY + height;
}

void Render(Cordinates c,const std::string& text){
    if(!visible) return;
    cords.x=c.x;
    cords.y=c.y;
    RenderText(text);
    //std::cout<<"Render label "<<cords.x<<" "<<text<<std::endl;




}

void Render(Cordinates c,Cordinates b){
    if(!visible) return;
    cords.x=c.x;
    cords.y=c.y;
    RenderBackground(b.x, b.y);
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
int getLabelW(){return  labelW;}


void setVisibility(bool b){visible=b;}
void setCords(int x,int y){


    cords.x=x;
    cords.y=y;
}

SDL_Color getTextColor(){


    return textColor;
}

SDL_Color getBackgroundColor(){


    return bg_color;
}

void setTextColor(SDL_Color c){

    textColor=c;
}

void setIconPositionLeft()  { iconPosition = IconPosition::LEFT; }
void setIconPositionRight() { iconPosition = IconPosition::RIGHT; }
void setBackgroundColor(SDL_Color c){

    bg_color=c;
}






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

    CDebugLabels(const CDebugLabels&) = delete;
    CDebugLabels& operator=(const CDebugLabels&) = delete;




    void Render(const std::vector<std::string>& strs){
        if(strs.empty()) return;

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


class CButton{

    private:
        std::unique_ptr<Clabel> label;
        Cordinates cords{0,0,};
        bool pressed=false,clickable=true,enabled=true,isLerping=false;
        SDL_Renderer * render;
        Cordinates mouseLoaction{0,0};
        SDL_Color nColor{76,76,76,128},hColor{128,128,128,128+64},startColor,targetColor,cColor;
        std::string Text;
        float lerpDuration=0,lerpProgress=0;




        Uint8 LerpChannel(Uint8 a, Uint8 b, float t) {
          return static_cast<Uint8>(a + (b - a) * t);
        }

    public:


    CButton(const std::string& text,SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){

        cords=c;
        label = std::make_unique<Clabel>(r,c,db,abs,v,f,tc);
        label->setBackgroundColor({64,64,64,64});
        cColor=nColor;
        Text=text;

    }





    void Render(){
        if(!enabled) return;
        //std::cout<<"----renderButton"<<std::endl;
        label->Render(cords,Text);


    }

    void Render(int x,int y){

        cords.x=x;
        cords.y=y;
        label->setCords(x, y);

        Render();
    }


    void Render(int x,int y,int w,int h){

        cords.x=x;
        cords.y=y;
        label->setCords(x, y);

       //if(!enabled) return;
        //std::cout<<"----renderButton"<<std::endl;
        label->Render(cords,Cordinates{w,h});
    }





    void setMouseLocation(int mX,int mY){
        if(!enabled) return;

        mouseLoaction.x=mX;
        mouseLoaction.y=mY;


    }


    bool CheckIfClicked(){

        if(!clickable || !enabled) return false;

       if((((cords.x+label->getLabelW())>mouseLoaction.x) && ((cords.x)<mouseLoaction.x)) &&  (((cords.y+label->getLabelH())>mouseLoaction.y) && ((cords.y)<mouseLoaction.y))){
        label->setBackgroundColor({128,128,128,128+64});

        //std::cout<<"----renderButton Click"<<std::endl;
        return true;
       }
    



       return false;
    }

    bool CheckIfHover(int x,int y,float dt){

        if(!clickable || !enabled) return false ;
        bool b=false;
        
       if((((cords.x+label->getLabelW())>x) && ((cords.x)<x)) &&  (((cords.y+label->getLabelH())>y) && ((cords.y)<y))){
        //label->setBackgroundColor(hColor);
        Update(dt);
        StartLerp(hColor,0.15f);
         b=true;
        
       // std::cout<<"----renderButton Click"<<std::endl;
       
       }else {
        Update(dt);
         StartLerp(nColor,0.15f);
        //label->setBackgroundColor(nColor);
       }
    


       return b;
    }

    void setSvgIcon(const char *filename,bool svgLeft,float scale=1 ){

        label->LoadSVGtoLabel(filename,scale);

        if(svgLeft) label->setIconPositionLeft();
        else label->setIconPositionRight();

    }

    void setEnabled(bool b){

        enabled=b;

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

        label->setBackgroundColor(cColor);
    }






    int getW(){return label->getLabelW();}
    int getH(){return label->getLabelH();}
    int getX(){return  cords.x;}
    int getY(){return  cords.y;}


    void setText(const std::string& s){

        Text=s;


    }

    void setnColor(SDL_Color c){

        nColor=c;
    }

    void sethColor(SDL_Color c){

        hColor=c;
    }



};




class CButtonHbox{
    private:

     std::vector<std::shared_ptr<CButton>> buttons;
     float width=0;

     public:





     void addButton(std::shared_ptr<CButton> btn){

        buttons.push_back(btn);


       



     }


    void Render(int x, int y){
        width=0;
        for(int i=0; i<buttons.size();i++){

            width+=buttons[i]->getW();
        }

        //std::cout<<"x "<<x<<" width x "<<x-width/2<<std::endl;
        float startX = x - width/2;
        float offset = 5;

        for (auto &b : buttons) {
               //std::cout<<"offset "<<offset<<std::endl;
            b->Render(startX + offset, y-b->getH());
            offset += b->getW();
        }
    }









};




class CAnimatedlabel{
    private:
    std::unique_ptr<Clabel> label;
    float ctime=0,time=0;
    std::string Text;

    Uint8 LerpChannel(Uint8 a, Uint8 b, float t) {
          return static_cast<Uint8>(a + (b - a) * t);
        }


    public:

    CAnimatedlabel(SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc,float anim_time, const std::string str){
        Text=str;
        label = std::make_unique<Clabel>(r,c,db,abs,v,f,tc);
        time=anim_time;
        ctime=0;



    }


    void Render(int x,int y,float dt){
        if(ctime<=0)return;

        label->Render({x,y},Text);
        ctime-=dt;

        SDL_Color tc= label->getTextColor();

        Uint8 na= LerpChannel(tc.a,0,ctime);

        label->setTextColor({tc.r,tc.g,tc.b,Uint8(255*ctime)});


    }

    void ResetTimer(){

        ctime=time;

    }

    

};





//  Clabel(SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){

//         renderer=r;
//         cords=c;
//         drawBackground=db;
//         absoluteCordinates=abs;
//         font=f;
//         visible=v;
//         textColor=tc;




//     }