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
#include "Cursor.hpp"

// not used
// WindowDecorations for borderless wayland issues
class CWindowDecorations{

    
    private:
    uint64_t size=50;
    int Y=5;

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
    SDL_Color Ui_color{0,0,0,255};



    public :


    CWindowDecorations(SDL_Renderer * r,TTF_Font *f,bool e,SDL_Color c){
        enabled=e;
        renderer=r;
        Ui_color=c;
        background = std::make_unique<Clabel>("________",renderer,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255});
        minimizeButton = std::make_unique<CButton>("",r,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255});
        maximizeButton = std::make_unique<CButton>("",r,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255});
        closeButton = std::make_unique<CButton>("",r,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255});
        settingsButton = std::make_unique<CButton>("",r,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255});
       
        
        background->setBackgroundColor({128,128,128,128});
        closeButton->setnColor({255,255,255,0});
        closeButton->sethColor({255,255,255,64});
        maximizeButton->setnColor({255,255,255,0});
        maximizeButton->sethColor({255,255,255,64});
        minimizeButton->setnColor({255,255,255,0});
        minimizeButton->sethColor({255,255,255,64});
        settingsButton->setnColor({255,255,255,0});
        settingsButton->sethColor({255,255,255,64});
    }


    void Render(int winW,int winH,int mouseX,int mouseY,float dt,CCursor::cursorType & cursor){

        if(!enabled) return;
        
        background->Render(Cordinates{0,0},Cordinates{winW,static_cast<int>(size)});
        closeButton->setText("Clo");
        closeButton->Render(winW-closeButton->getW(),Y);
       if( closeButton->CheckIfHover(mouseX, mouseY, dt)) cursor = CCursor::Hand;

        maximizeButton->Render(closeButton->getX()-maximizeButton->getW(),Y+2);
       if(  maximizeButton->CheckIfHover(mouseX, mouseY, dt)) cursor = CCursor::Hand;

        minimizeButton->Render(maximizeButton->getX()-minimizeButton->getW(),Y+5);
       if( minimizeButton->CheckIfHover(mouseX, mouseY, dt))cursor = CCursor::Hand;



        settingsButton->Render(0,Y);
        if( settingsButton->CheckIfHover(mouseX, mouseY, dt))cursor = CCursor::Hand;;

        size = closeButton->getH();


    }

    bool CheckifCloseClick(int mX,int mY){

        closeButton->setMouseLocation(mX,mY);
        return closeButton->CheckIfClicked();



    }

    bool CheckifMinimizeClick(int mX,int mY){

        minimizeButton->setMouseLocation(mX,mY);
        return minimizeButton->CheckIfClicked();



    }

    bool CheckifMaximizeClick(int mX,int mY){

        maximizeButton->setMouseLocation(mX,mY);
        return maximizeButton->CheckIfClicked();



    }

    bool CheckifOptionClick(int mX,int mY){

        settingsButton->setMouseLocation(mX,mY);
        return settingsButton->CheckIfClicked();



    }

    void setBackgroundColor(SDL_Color c){


        background->setBackgroundColor(c);
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
       minimizeButton->setSvgIcon(minimizeSVG.c_str(),false,Ui_color,0.028f);
    }

    void SetMaximizeSVG(const std::string& value) {
        maximizeSVG = value;
        maximizeButton->setSvgIcon(maximizeSVG.c_str(),false,Ui_color,0.03f);
    }

    void SetCloseSVG(const std::string& value) {
        closeSVG = value;
        closeButton->setSvgIcon(closeSVG.c_str(),false,Ui_color,0.032f);
    }

    void SetOptionSVG(const std::string& value) {
        optionSVG = value;
        settingsButton->setSvgIcon(optionSVG.c_str(),false,Ui_color,0.032f);
    }


};
