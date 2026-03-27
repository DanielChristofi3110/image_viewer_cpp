#pragma once
#include "globals.hpp"
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL2_rotozoom.h>
#include <SDL2/SDL_surface.h>

#include <SDL2/SDL_ttf.h>
#include <cstdint>
#include <memory>
#include "GUI.hpp"


class CWindowDecorations{

    
    private:
    uint64_t size=50;
    int Y=0;

    std::unique_ptr<Clabel> background;
    std::unique_ptr<CButton> minimizeButton;
    std::unique_ptr<CButton> maximizeButton;
    std::unique_ptr<CButton> closeButton;
    std::unique_ptr<CButton> settingsButton;
    SDL_Renderer * renderer;
    bool enabled =true;
    std::string minimizeSVG="";
    std::string maximizeSVG="";
    std::string closeSVG="";
    std::string optionSVG="";



    public :

    //Clabel(const std::string txt, SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){
    //CButton(const std::string& text,SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){

    CWindowDecorations(SDL_Renderer * r,TTF_Font *f,bool e){
        enabled=e;
        renderer=r;
        background = std::make_unique<Clabel>("________",renderer,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255});
        minimizeButton = std::make_unique<CButton>("Min",r,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255});
        maximizeButton = std::make_unique<CButton>("Max",r,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255});
        closeButton = std::make_unique<CButton>("",r,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255});
        settingsButton = std::make_unique<CButton>("Set",r,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255});
       
        
        background->setBackgroundColor({128,128,128,128});
        closeButton->setnColor({255,0,0,0});
        closeButton->sethColor({255,0,0,0});
        maximizeButton->setnColor({255,0,0,0});
        maximizeButton->sethColor({255,0,0,0});
        minimizeButton->setnColor({255,0,0,0});
        minimizeButton->sethColor({255,0,0,0});
        settingsButton->setnColor({255,0,0,0});
        settingsButton->sethColor({255,0,0,0});
    }


    void Render(int winW,int winH,int mouseX,int mouseY,float dt){

        if(!enabled) return;
        
        background->Render(Cordinates{0,Y},Cordinates{winW,static_cast<int>(size)});
        closeButton->setText("Clo");
        closeButton->Render(winW-closeButton->getW(),Y);
        closeButton->CheckIfHover(mouseX, mouseY, dt);

        //maximizeButton->setText("Max");
        maximizeButton->Render(closeButton->getX()-maximizeButton->getW(),Y);
        maximizeButton->CheckIfHover(mouseX, mouseY, dt);

       // minimizeButton->setText("Min");
        minimizeButton->Render(maximizeButton->getX()-minimizeButton->getW(),Y);
        minimizeButton->CheckIfHover(mouseX, mouseY, dt);



       // settingsButton->setText("Set");
        settingsButton->Render(0,Y);
        settingsButton->CheckIfHover(mouseX, mouseY, dt);

        size = closeButton->getH();


    }


    uint64_t getSize(){
        if(!enabled) return 0;
        return size;
    }

    int getH(){
        if(!enabled) return 0;
        return  static_cast<int>(size);
    }


    void SetMinimizeSVG(const std::string& value) {
       minimizeSVG = value;
    }

    void SetMaximizeSVG(const std::string& value) {
        maximizeSVG = value;
    }

    void SetCloseSVG(const std::string& value) {
        closeSVG = value;
        closeButton->setSvgIcon(closeSVG.c_str(),false,0.04f);
    }

    void SetOptionSVG(const std::string& value) {
        optionSVG = value;
    }


};
