#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <cstdint>
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include "SimpleIni.h"
#include "GUI.hpp"
#include "Cursor.hpp"
#include "globals.hpp"

class CConfigLoader
{
private:
    std::string fontName;
    int fontSize = 0;
    int idleFps = 0;
    
    int aSYNCLOADING= 1;
    int uNLOADAT= 2;
    int mAXIMAGE_QUEUE=10;
    bool hIDE_UI;

public:
    bool load(const std::string& filename)
    {
        CSimpleIniA ini;
        ini.SetUnicode();

        SI_Error rc = ini.LoadFile(filename.c_str());
        if (rc < 0)
        {
            std::cerr << "Failed to load config file\n";
            return false;
        }

        const char* name = ini.GetValue("font", "name", "InterVariable.ttf");
        long size = ini.GetLongValue("font", "size", 18);
        long fps = ini.GetLongValue("settings", "idleFps", 10);
        long as = ini.GetLongValue("settings", "ASYNCLOADING", 1);
        long un = ini.GetLongValue("settings", "UNLOADAT", 2);
        long ma = ini.GetLongValue("settings", "MAXIMAGE_QUEUE", 10);
        long hu = ini.GetLongValue("settings", "HIDE_UI", 0);

        fontName = name;
        fontSize = static_cast<int>(size);
        idleFps =static_cast<int>(fps);
        aSYNCLOADING= static_cast<int>(as);
        uNLOADAT= static_cast<int>(un);
        mAXIMAGE_QUEUE=static_cast<int>(ma);
        hIDE_UI=static_cast<bool>(hu);

        return true;
    }

    bool save(const std::string& filename){
        CSimpleIniA ini;
        ini.SetUnicode();

        // Load existing file (optional but recommended)
        ini.LoadFile(filename.c_str());

        ini.SetValue("font", "name", fontName.c_str());
        ini.SetLongValue("font", "size", fontSize);
        ini.SetLongValue("settings", "idleFps", idleFps);
        ini.SetLongValue("settings", "ASYNCLOADING", aSYNCLOADING);
        ini.SetLongValue("settings", "UNLOADAT", uNLOADAT);
        ini.SetLongValue("settings", "MAXIMAGE_QUEUE", mAXIMAGE_QUEUE);
         ini.SetLongValue("settings", "HIDE_UI", hIDE_UI);

        SI_Error rc = ini.SaveFile(filename.c_str());
        return rc >= 0;
    }



    const std::string& getFontName() const
    {
        return fontName;
    }

    int getFontSize() const
    {
        return fontSize;
    }
     int getidleFps() const
    {
        return idleFps;
    }

     int getASYNCLOADING() const
    {
        return aSYNCLOADING;
    }

      int getUNLOADAT() const
    {
        return uNLOADAT;
    }
       int getMAXIMAGE_QUEUE() const
    {
        return mAXIMAGE_QUEUE;
    }

    bool getHIDE_UI() const
    {
        return hIDE_UI;
    }

    void setFontName(const std::string& v) { fontName = v; }
    void setFontSize(int v) { fontSize = v; }
    void setIdleFps(int v) { idleFps = v; }
    void setASYNCLOADING(int v) { aSYNCLOADING = v; }
    void setUNLOADAT(int v) { uNLOADAT = v; }
    void setMAXIMAGE_QUEUE(int v) { mAXIMAGE_QUEUE = v; }
     void setHIDE_UI(bool v) { hIDE_UI = v; }

};


class CConfigEditorGUI{
    private:
    SDL_Renderer * renderer;
    bool visible=true,enabled=true;
    std::unique_ptr<Clabel> back_lable;
    std::vector<std::unique_ptr<Clabel>> labels;
    std::vector<std::unique_ptr<CTextBox>> TextBoxes;
    std::unique_ptr<CButton> SaveButton;
    std::vector<std::string> FieldName = {
    "Font Name",
    "Font Size",
    "Idle FPS",
    "Async Loading",
    "Unload At",
    "Max Image Queue",
    "HIDE_UI"
        };
    bool AnyTyping=false;
    Cordinates cords{100,100};
    CConfigLoader *cfg;

    



    public:


    // Clabel(SDL_Renderer* r,Cordinates c,bool db, bool abs,TTF_Font * f){
    // CTextBox(SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){
    // CButton(const std::string& text,SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){
    CConfigEditorGUI(SDL_Renderer *r,TTF_Font * f, CConfigLoader* _cfg){

        renderer=r;

        cfg=_cfg;
        
        back_lable=std::make_unique<Clabel>(r,Cordinates{300,300},true,true,f);

        back_lable->setTextColor({0,0,0,0});
        back_lable->setBackgroundColor({128,128,128,128});

        SaveButton = std::make_unique<CButton>("Save",renderer,Cordinates{500,500},true,true,true,f,SDL_Color{255,255,255,255});



    // FieldName.push_back(("Test1"));
    // FieldName.push_back(("Test2"));
    // FieldName.push_back(("Test3")); 


    for(uint64_t i=0;i<FieldName.size() ;i++){
        labels.push_back((std::make_unique<Clabel>(r,Cordinates{300,300},true,true,f)));
        TextBoxes.push_back((std::make_unique<CTextBox>(r,Cordinates{200,200},true,true,true,f,SDL_Color{255,255,255,255})));
        //TextBoxes[i]->setText("std::string s");
        TextBoxes[i]->textType=CTextBox::NumOnly;
    }

    TextBoxes[0]->textType=CTextBox::TextOnly;

    }
    void loadFromConfig(){
            TextBoxes[0]->setText(cfg->getFontName());
            TextBoxes[1]->setText(std::to_string(cfg->getFontSize()));
            TextBoxes[2]->setText(std::to_string(cfg->getidleFps()));
            TextBoxes[3]->setText(std::to_string(cfg->getASYNCLOADING()));
            TextBoxes[4]->setText(std::to_string(cfg->getUNLOADAT()));
            TextBoxes[5]->setText(std::to_string(cfg->getMAXIMAGE_QUEUE()));
            TextBoxes[6]->setText(std::to_string(cfg->getHIDE_UI()));
        }

    void Render(int wW,int wH,int mx,int my,float dt,CCursor::cursorType &cursor){
        if (!visible || !enabled) return;

        back_lable->Render(Cordinates{0,0},Cordinates{wW,wH});

        int inity=cords.y;
        int initx=cords.x;
        for(uint64_t i=0; i<TextBoxes.size(); i++){

        labels[i]->Render({initx,inity},FieldName[i]);
        TextBoxes[i]->setCords(initx+labels[i]->getLabelW(), inity);
        TextBoxes[i]->Render(wW, wH, mx, my,dt, cursor);

        inity+=40;
        }
        if(SaveButton->CheckIfHover(mx, my, dt)) cursor=CCursor::Hand;
        SaveButton->Render(initx,inity);
        


    }

    void checkIfAnyTyping(int mx,int my){

        if(TextBoxes.empty()) return;
        AnyTyping=false;

        for(uint64_t i=0; i<TextBoxes.size(); i++){
            TextBoxes[i]->CheckIfPressed(mx, my, AnyTyping);

        }

    }

    // "Font Name", 0
    // "Font Size", 1
    // "Idle FPS", 2
    // "Async Loading", 3
    // "Unload At", 4
    // "Max Image Queue" 5
    void checkIfSave(int mx,int my,const std::string& filename){

       SaveButton->setMouseLocation(mx, my); 
       if( SaveButton->CheckIfClicked()){
        std::cout<<"SavePressConf"<<std::endl;

        cfg->setFontName(TextBoxes[0]->getText());
        cfg->setFontSize(TextBoxes[1]->getInt());
        cfg->setIdleFps(TextBoxes[2]->getInt());
        cfg->setASYNCLOADING(TextBoxes[3]->getInt());
        cfg->setUNLOADAT(TextBoxes[4]->getInt());
        cfg->setMAXIMAGE_QUEUE(TextBoxes[5]->getInt());
        cfg->setHIDE_UI(static_cast<bool>(TextBoxes[6]->getInt()));

         cfg->save(filename);
    }
       return;
    }

    void AddTextToTyping(const std::string s){

        if(!AnyTyping) return;

        for(uint64_t i=0; i<TextBoxes.size(); i++){
            if(TextBoxes[i]->isFocused()){

                TextBoxes[i]->appendtText(s);
            }

        }
    }

     void POPTextToTyping(){

        if(!AnyTyping) return;

        for(uint64_t i=0; i<TextBoxes.size(); i++){
            if(TextBoxes[i]->isFocused()){

                TextBoxes[i]->popText();
            }

        }
    }

    void setEnabled(bool b){

        enabled=b;
    }

    bool isEnabled(){

        return enabled;
    }

};