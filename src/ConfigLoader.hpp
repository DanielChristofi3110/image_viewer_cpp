#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <cstdint>
#include <iterator>
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
    std::string version;
    int fontSize = 18;
    int idleFps = 24;

    int aSYNCLOADING = 4;
    int uNLOADAT = 4;
    int mAXIMAGE_QUEUE = 10;
    int tHUMBNAIL_ANIMATION_SPEED = 5;
    bool hIDE_UI = false;
    bool single_image_load =false;
    SDL_Color mainColor;

public:
    void setDefaults()
    {
        fontName = "InterVariable.ttf";
        fontSize = 18;
        idleFps = 24;
        aSYNCLOADING = 4;
        uNLOADAT = 4;
        mAXIMAGE_QUEUE = 10;
        hIDE_UI = false;
        tHUMBNAIL_ANIMATION_SPEED = 5;
    }

    bool load(const std::string& filename)
    {
        CSimpleIniA ini;
        ini.SetUnicode();


         if(DEBUG) std::cout <<filename.c_str()<< "\n";
        // If file doesn't exist → create default
        if (!std::filesystem::exists(filename))
        {
            std::cout << "Config not found. Creating default config...\n";
            setDefaults();
            return save(filename);
        }

        SI_Error rc = ini.LoadFile(filename.c_str());
        if (rc < 0)
        {
            std::cerr << "Failed to load config. Recreating default..."<<filename.c_str()<<"\n";
            setDefaults();
            return save(filename);
        }

        const char* name = ini.GetValue("font", "name", "InterVariable.ttf");
        long size = ini.GetLongValue("font", "size", 18);
        long fps = ini.GetLongValue("settings", "idleFps", 24);
        long as = ini.GetLongValue("settings", "ASYNCLOADING", 4);
        long un = ini.GetLongValue("settings", "UNLOADAT", 4);
        long ma = ini.GetLongValue("settings", "MAXIMAGE_QUEUE", 10);
        long hu = ini.GetLongValue("settings", "HIDE_UI", 0);
        long si = ini.GetLongValue("settings", "SINGLE_IMAGE_LOAD", 0);

        long ta = ini.GetLongValue("settings", "THUMBNAIL_ANIMATION_SPEED", 5);
        const char* ver = ini.GetValue("info", "version", "none");


        long mcR= ini.GetLongValue("theme", "MainR", 255);
        long mcG= ini.GetLongValue("theme", "MainG", 255);
        long mcB= ini.GetLongValue("theme", "MainB", 255);

        // if(  ver APP_VERSION){
        //     version=APP_VERSION;
        //     setDefaults();
        //     return save(filename);

        // }

        mainColor= {static_cast<Uint8>(mcR),static_cast<Uint8>(mcG),static_cast<Uint8>(mcB),255};
        fontName = name;
        fontSize = static_cast<int>(size);
        idleFps = static_cast<int>(fps);
        aSYNCLOADING = static_cast<int>(as);
        uNLOADAT = static_cast<int>(un);
        mAXIMAGE_QUEUE = static_cast<int>(ma);
        hIDE_UI = static_cast<bool>(hu);
        single_image_load= static_cast<bool>(si);
        tHUMBNAIL_ANIMATION_SPEED = static_cast<int>(ta);

        return true;
    }

    bool save(const std::string& filename)
    {
        CSimpleIniA ini;
        ini.SetUnicode();

        // Ensure directory exists
        std::filesystem::create_directories(
            std::filesystem::path(filename).parent_path()
        );

        ini.SetValue("font", "name", fontName.c_str());
        ini.SetLongValue("font", "size", fontSize);

        ini.SetLongValue("settings", "idleFps", idleFps);
        ini.SetLongValue("settings", "ASYNCLOADING", aSYNCLOADING);
        ini.SetLongValue("settings", "UNLOADAT", uNLOADAT);
        ini.SetLongValue("settings", "MAXIMAGE_QUEUE", mAXIMAGE_QUEUE);
        ini.SetLongValue("settings", "HIDE_UI", hIDE_UI);
        ini.SetLongValue("settings", "SINGLE_IMAGE_LOAD", single_image_load);

        ini.SetLongValue("settings", "THUMBNAIL_ANIMATION_SPEED", tHUMBNAIL_ANIMATION_SPEED);

        ini.SetValue("info", "version", version.c_str());

        ini.SetLongValue("theme", "MainR", mainColor.r);
        ini.SetLongValue("theme", "MainG", mainColor.g);
        ini.SetLongValue("theme", "MainB", mainColor.b);

        SI_Error rc = ini.SaveFile(filename.c_str());
        return rc >= 0;
    }

    // --- getters ---
    const std::string& getFontName() const { return fontName; }
    int getFontSize() const { return fontSize; }
    int getidleFps() const { return idleFps; }
    int getASYNCLOADING() const { return aSYNCLOADING; }
    int getUNLOADAT() const { return uNLOADAT; }
    int getMAXIMAGE_QUEUE() const { return mAXIMAGE_QUEUE; }
    bool getHIDE_UI() const { return hIDE_UI; }
    float getTHUMBNAIL_ANIMATION_SPEED() const { return tHUMBNAIL_ANIMATION_SPEED; }
    SDL_Color getMainColor()const {return mainColor;}
    bool getSingle_image_load() const{return single_image_load;}

    // --- setters ---
    void setFontName(const std::string& v) { fontName = v; }
    void setFontSize(int v) { fontSize = v; }
    void setIdleFps(int v) { idleFps = v; }
    void setASYNCLOADING(int v) { aSYNCLOADING = v; }
    void setUNLOADAT(int v) { uNLOADAT = v; }
    void setMAXIMAGE_QUEUE(int v) { mAXIMAGE_QUEUE = v; }
    void setHIDE_UI(bool v) { hIDE_UI = v; }
    void setTHUMBNAIL_ANIMATION_SPEED(float v) { tHUMBNAIL_ANIMATION_SPEED = v; }
    void setMainColor(SDL_Color c){mainColor=c;}
      void setSingle_image_load (bool v) { single_image_load = v; }
};

class CConfigEditorGUI{
    private:
    SDL_Renderer * renderer;
    bool visible=true,enabled=true;
    std::unique_ptr<Clabel> back_lable;
    std::vector<std::unique_ptr<Clabel>> labels;
    std::vector<std::unique_ptr<CTextBox>> TextBoxes;
    std::unique_ptr<CDropDown> FontSelectorDropDown;
    std::unique_ptr<CButton> SaveButton;
    std::unique_ptr<CButton> ExitButton;
    std::vector<std::string> FieldName = {
    "Font Size",
    "Idle FPS",
    "Async Loading",
    "Unload At",
    "Max Image Queue",
    "HIDE_UI",
    "THUMBNAIL_ANIMATION_SPEED",
    "MainR",
    "MainG",
    "MainB",
   "SINGLE_IMAGE_LOAD",
   APP_VERSION
        };
    bool AnyTyping=false;
    Cordinates cords{100,100};
    std::shared_ptr<CConfigLoader> cfg;
    int Yspace=40;
    int Xspace=40;


    



    public:


    // Clabel(SDL_Renderer* r,Cordinates c,bool db, bool abs,TTF_Font * f){
    // CTextBox(SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){
    // CButton(const std::string& text,SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){
    //CDropDown(Cordinates c,SDL_Renderer* r,bool db, bool abs,bool v,TTF_Font * f){
    CConfigEditorGUI(SDL_Renderer *r,TTF_Font * f,  std::shared_ptr<CConfigLoader> _cfg){

        renderer=r;

        cfg=_cfg;
        

        std::vector<std::string> DropDownItems={
            "InterVariable.ttf",
            "SFUIDisplay-Light.ttf",
            "LeagueScriptNumberOne-webfont.ttf"
            

        };
        FontSelectorDropDown = std::make_unique<CDropDown>(Cordinates{0,0},renderer,true,true,true,f,DropDownItems,0);


        back_lable=std::make_unique<Clabel>(r,Cordinates{300,300},true,true,f);

        back_lable->setTextColor({0,0,0,0});
        back_lable->setBackgroundColor({128,128,128,128});

        SaveButton = std::make_unique<CButton>("Save",renderer,Cordinates{500,500},true,true,true,f,SDL_Color{255,255,255,255});
        ExitButton = std::make_unique<CButton>("Exit",renderer,Cordinates{500,500},true,true,true,f,SDL_Color{255,255,255,255});



    // FieldName.push_back(("Test1"));
    // FieldName.push_back(("Test2"));
    // FieldName.push_back(("Test3")); 


    //labels.push_back((std::make_unique<Clabel>(r,Cordinates{300,300},true,true,f)));
    //labels[0]->setText("Font Type:");
    for(uint64_t i=0;i<FieldName.size() ;i++){
        labels.push_back((std::make_unique<Clabel>(r,Cordinates{300,300},true,true,f)));
        TextBoxes.push_back((std::make_unique<CTextBox>(r,Cordinates{200,200},true,true,true,f,SDL_Color{255,255,255,255})));
        //TextBoxes[i]->setText("std::string s");
        TextBoxes.back()->textType=CTextBox::NumOnly;
    }

   // TextBoxes[0]->textType=CTextBox::TextOnly;

    }
    void loadFromConfig(){
           // TextBoxes[0]->setText(cfg->getFontName());
            FontSelectorDropDown->setCurrentSelectionByString(cfg->getFontName());
            TextBoxes[0]->setText(std::to_string(cfg->getFontSize()));
            TextBoxes[1]->setText(std::to_string(cfg->getidleFps()));
            TextBoxes[2]->setText(std::to_string(cfg->getASYNCLOADING()));
            TextBoxes[3]->setText(std::to_string(cfg->getUNLOADAT()));
            TextBoxes[4]->setText(std::to_string(cfg->getMAXIMAGE_QUEUE()));
            TextBoxes[5]->setText(std::to_string(cfg->getHIDE_UI()));
            TextBoxes[6]->setText(std::to_string(static_cast<int>(cfg->getTHUMBNAIL_ANIMATION_SPEED())));
            TextBoxes[7]->setText(std::to_string(cfg->getMainColor().r));
            TextBoxes[8]->setText(std::to_string(cfg->getMainColor().g));
            TextBoxes[9]->setText(std::to_string(cfg->getMainColor().b));
            TextBoxes[10]->setText(std::to_string(cfg->getSingle_image_load()));
        }

    void Render(int wW,int wH,int mx,int my,float dt,CCursor::cursorType &cursor){
        if (!visible || !enabled) return;

        back_lable->Render(Cordinates{0,0},Cordinates{wW,wH});

        int inity=cords.y;
        int initx=cords.x;

      

        inity+=Yspace;
        for(uint64_t i=0; i<TextBoxes.size(); i++){

        labels[i]->Render({initx,inity},FieldName[i]);
        TextBoxes[i]->setCords(initx+labels[i]->getLabelW(), inity);
        if(FontSelectorDropDown->isColapsed())TextBoxes[i]->Render(wW, wH, mx, my,dt, cursor);
        else TextBoxes[i]->Render(wW, wH, mx, my,dt);

        inity+=Yspace;
        }
        if(SaveButton->CheckIfHover(mx, my, dt)) cursor=CCursor::Hand;
        if(ExitButton->CheckIfHover(mx, my, dt)) cursor=CCursor::Hand;
        
        SaveButton->Render(initx,inity);
        ExitButton->Render(initx+Xspace+SaveButton->getW(),inity);
        FontSelectorDropDown->Render(cords.x, cords.y, mx,my, dt,cursor);


    }

    void checkIfAnyTyping(int mx,int my){

        if(TextBoxes.empty() || !FontSelectorDropDown->isColapsed()) return;
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
    void checkIfButtonClick(int mx,int my,const std::string& filename){

        FontSelectorDropDown->CheckIfClickedCurrentSelection(mx, my);
        if(!FontSelectorDropDown->isColapsed()){
        if(DEBUG) std::cout<<FontSelectorDropDown->CheckIfClickedOption(mx, my)<<std::endl;
         int ci=FontSelectorDropDown->CheckIfClickedOption(mx, my);

         if(ci!=-1){
            FontSelectorDropDown->setCurrentSelection(static_cast<uint64_t>(ci));
        
        }
    }

       SaveButton->setMouseLocation(mx, my); 
       if( SaveButton->CheckIfClicked()){
        if(DEBUG) std::cout<<"SavePressConf"<<std::endl;

        cfg->setFontName(FontSelectorDropDown->getCurrentSelectionString());
        cfg->setFontSize(TextBoxes[0]->getInt());
        cfg->setIdleFps(TextBoxes[1]->getInt());
        cfg->setASYNCLOADING(TextBoxes[2]->getInt());
        cfg->setUNLOADAT(TextBoxes[3]->getInt());
        cfg->setMAXIMAGE_QUEUE(TextBoxes[4]->getInt());
        cfg->setHIDE_UI(static_cast<bool>(TextBoxes[5]->getInt()));
        cfg->setTHUMBNAIL_ANIMATION_SPEED(static_cast<int>(TextBoxes[6]->getInt()));
        cfg->setMainColor({static_cast<Uint8>(TextBoxes[7]->getInt()),static_cast<Uint8>(TextBoxes[8]->getInt()),static_cast<Uint8>(TextBoxes[9]->getInt())});
        cfg->setSingle_image_load(TextBoxes[10]->getInt());
        cfg->save(filename);
    }

       ExitButton->setMouseLocation(mx, my); 
       if( ExitButton->CheckIfClicked()){
        if(DEBUG) std::cout<<"ExitPressConf"<<std::endl;
        //visible=false;
        enabled=false;
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