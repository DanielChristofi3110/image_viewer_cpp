#pragma once
#include "globals.hpp"
#include "Cursor.hpp"
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
#include <vector>

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"



//label
class Clabel{
    private:
    Cordinates cords{0,0};
    SDL_Renderer* renderer=nullptr;
    bool drawBackground=false,absoluteCordinates=false,visible=true;
    SDL_Color textColor{255,255,255,255},bg_color{76, 76, 76, 150};
    TTF_Font* font=nullptr;
    SDL_Texture* texture=nullptr;
    SDL_Texture* textTexture=nullptr;
    std::string Text;
    int Nexty;
    int labelH=0,labelW=0;
    int textH=0,textW=0;
    
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
                std::cout << "Failed to load SVG "<<filename<<"\n";
                return nullptr;
            }

            int width  = std::round(image->width * scale);
            int height = std::round(image->height * scale);

            
            NSVGrasterizer* rast = nsvgCreateRasterizer();

            
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

    SDL_Texture* LoadSVG(SDL_Renderer* renderer, const char* filename, float scale, SDL_Color color)
    {
        NSVGimage* image = nsvgParseFromFile(filename, "px", 96.0f);
        if (!image) {
            std::cout << "Failed to load SVG " << filename << "\n";
            return nullptr;
        }

        int width  = std::round(image->width * scale);
        int height = std::round(image->height * scale);

        NSVGrasterizer* rast = nsvgCreateRasterizer();

        std::vector<unsigned char> img(width * height * 4, 0);

        nsvgRasterize(rast, image, 0, 0, scale, img.data(), width, height, width * 4);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * 4;

                unsigned char alpha = img[idx + 3];

                img[idx + 0] = color.r; 
                img[idx + 1] = color.g; 
                img[idx + 2] = color.b; 
                img[idx + 3] = alpha;  
            }
        }

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
    Clabel(const std::string txt, SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){

        renderer=r;
        cords=c;
        drawBackground=db;
        absoluteCordinates=abs;
        font=f;
        visible=v;
        textColor=tc;
        setText(txt);



    }

    ~Clabel(){
        if(texture) SDL_DestroyTexture(texture);
        if (textTexture)SDL_DestroyTexture(textTexture);
        if(DEBUG) std::cout<<"Destroy Label "<<std::endl;
    }

    Clabel(const Clabel&) = delete;
    Clabel& operator=(const Clabel&) = delete;

void setText(const std::string& text){

    textW = 0;
    textH = 0;
    SDL_Surface* textSurface = nullptr;
    if(textTexture!=nullptr) SDL_DestroyTexture(textTexture);


    if (!text.empty())
        {
            Text=text;
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


}


void Render()
{
    if (!visible) return;
    if (!font || !renderer) return;



    int totalWidth = textW;

    if (texture)
    {
        if (Text.empty())
            totalWidth += iconWidth;
        else
            totalWidth += iconWidth + spacing;
    }

    int contentHeight = std::max(textH, texture ? iconHeight : 0);

    int baseY = absoluteCordinates ? cords.y : cords.y - contentHeight;

    int currentX = cords.x;

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

    if (texture && iconPosition == IconPosition::LEFT)
    {
        SDL_Rect iconRect;
        iconRect.x = currentX;
        iconRect.y = baseY + (contentHeight - iconHeight) / 2;
        iconRect.w = iconWidth;
        iconRect.h = iconHeight;

        SDL_RenderCopy(renderer, texture, nullptr, &iconRect);

        currentX += iconWidth + (Text.empty()? 0 : spacing);
    }

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


}





void RenderBackground(int width, int height)
{
    if (!visible) return;
    if (!renderer) return;
    if (!drawBackground) return;

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
    setText(text);
    Render();




}

void Render(Cordinates c){
    if(!visible) return;
    cords.x=c.x;
    cords.y=c.y;
    Render();




}

void Render(Cordinates c,Cordinates b){
    if(!visible) return;
    cords.x=c.x;
    cords.y=c.y;
    RenderBackground(b.x, b.y);




}

void Render(const std::string& text){
    if(!visible) return;
    setText(text);
    Render();




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

void LoadSVGtoLabel(const char* filename,SDL_Color color, float scale = 1.0f)
{
    if (texture)
        SDL_DestroyTexture(texture);

    texture = LoadSVG(renderer, filename, scale,color);

    if (texture)
    {
        SDL_QueryTexture(texture, nullptr, nullptr, &iconWidth, &iconHeight);
    }
}




int getNexty(){return Nexty;}
bool  isVisible(){return visible;}
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




//Debug Ui
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
        labels[0]->setText(strs[0]);
        labels[0]->Render();

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

// Button
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
        label->setText(Text);

    }





    void Render(){
        if(!enabled) return;
        label->Render(cords);


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

        label->Render(cords,Cordinates{w,h});
    }

    void UpdateText(){


        label->setText(Text);

   
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

        return true;
       }
    



       return false;
    }

    bool CheckIfHover(int x,int y,float dt){

        if(!clickable || !enabled) return false ;
        bool b=false;
        
       if((((cords.x+label->getLabelW())>x) && ((cords.x)<x)) &&  (((cords.y+label->getLabelH())>y) && ((cords.y)<y))){
        Update(dt);
        StartLerp(hColor,0.15f);
         b=true;
        
       
       }else {
        Update(dt);
         StartLerp(nColor,0.15f);
       }
    


       return b;
    }

    void setSvgIcon(const char *filename,bool svgLeft,float scale=1 ){

        label->LoadSVGtoLabel(filename,scale);

        if(svgLeft) label->setIconPositionLeft();
        else label->setIconPositionRight();

    }
    void setSvgIcon(const char *filename,bool svgLeft,SDL_Color c,float scale=1 ){

        label->LoadSVGtoLabel(filename,c,scale);

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



//Button Hbox
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

        float startX = x - width/2;
        float offset = 5;

        for (auto &b : buttons) {
            b->Render(startX + offset, y-b->getH());
            offset += b->getW();
        }
    }









};



//fading label
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
        

        SDL_Color tc= label->getTextColor();
        SDL_Color bc= label->getBackgroundColor();

        
        
        if(ctime>0){
        label->setTextColor({tc.r,tc.g,tc.b,Uint8(255*(ctime/time))});
        label->setBackgroundColor({0,0,0,Uint8(255*(ctime/time))});
        }
        if(dt<0.5){
        ctime-=dt;
        }


    }

    void ResetTimer(){

        ctime=time;

    }

    

};


// Under Mouse label
class CMouseLabel{

private:

Cordinates Offset {0,0};
std::unique_ptr<Clabel> label;
std::string Text="";
public:

CMouseLabel(SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){

    label=std::make_unique<Clabel>(r,c,db,abs,v,f,tc);
    label->setBackgroundColor({0,0,0,255});
    label->setTextColor({255,255,255,255});




}


void Render(int mouseX,int mouseY,int winW,int winH,const std::string txt){

    if(mouseX>winW-label->getLabelW()){
        Offset.x=-label->getLabelW();
    }else {
         Offset.x=0;
    }

    label->Render({mouseX+Offset.x,mouseY+Offset.y},txt);
}






};

// Text Box
class CTextBox{
    private:
    std::unique_ptr<Clabel> label_back;
    std::unique_ptr<Clabel> label_text;
    std::unique_ptr<Clabel> label_blink;
    Cordinates cords{100,100};
    bool clickable=true,enabled =true,visible=true,focused=false,blink_phase=false;
    float blink=0.3,cblink=0;
    std::string Text="";
    public:
    enum TextType {
        String,
        TextOnly,
        NumOnly
    };
    
    TextType textType=TextType::String;


    CTextBox(SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){



    cords =c;

    label_back=std::make_unique<Clabel>(r,c,db,abs,v,f,tc);
    label_back->setBackgroundColor({128,128,128,255});
    label_back->setTextColor({255,255,255,0});


    label_blink=std::make_unique<Clabel>(r,c,db,abs,v,f,tc);
    label_blink->setBackgroundColor({255,255,255,255});
    label_blink->setTextColor({255,255,255,0});

    label_text=std::make_unique<Clabel>(r,c,true,abs,v,f,tc);
    label_text->setBackgroundColor({128,128,128,0});
    label_text->setTextColor({255,255,255,0});




}

    void Render(int wW,int wH,int mx,int my ,float dt,CCursor::cursorType & cursor){
        if (!visible) return;

        label_back->Render(cords,Cordinates{label_text->getLabelW()<100?100:label_text->getLabelW(),20});
        if(CheckIfHover(mx,my,0.f)) cursor =CCursor::IBeam;
        
        label_text->Render(cords,Text);
     
        UpdateBlink(label_text->getLabelW(),dt);

        




    }

    void Render(int wW,int wH,int mx,int my ,float dt){
        if (!visible) return;

        label_back->Render(cords,Cordinates{label_text->getLabelW()<100?100:label_text->getLabelW(),20});
        CheckIfHover(mx,my,0.f);
        
        label_text->Render(cords,Text);
     
        UpdateBlink(label_text->getLabelW(),dt);

        




    }
    void setText(std::string s){


        Text=s;
    }
    void setCords(int x,int y){

        cords={x,y};
    }
    void appendtText(const std::string s){
        if(!visible || !enabled || !clickable) return;

        switch (textType) {

            case String:
            Text+=s;

            break;

            case TextOnly:
            for (char c : s) if (!isdigit(c)) Text+=c;

            

            break;


            case NumOnly: for (char c : s) if (isdigit(c)) Text+=c;

            break;
        
        }
       
    }
    void popText(){

         if (!Text.empty()) Text.pop_back();
    }
    void UpdateBlink(int offx,float dt){

        if(!focused) return;
         cblink-=dt;

        if(cblink<=0){
        cblink=blink;
        blink_phase=!blink_phase;

        }

        if(blink_phase) label_blink->Render(Cordinates{offx>10?cords.x+offx-10:cords.x,cords.y},Cordinates{3,20});


    }
    
    bool CheckIfHover(int x,int y,float dt){

        if(!clickable || !enabled) return false ;
        bool b=false;
        
       if((((cords.x+label_back->getLabelW())>x) && ((cords.x)<x)) &&  (((cords.y+label_back->getLabelH())>y) && ((cords.y)<y))){
         b=true;
        
       
       }else {
       }
    


       return b;
    }


     void CheckIfPressed(int x,int y,bool & lock){
        
        if(!clickable || !enabled || !visible) {
            
            
            return ;}
       
        focused=false;
       if((((cords.x+label_back->getLabelW())>x) && ((cords.x)<x)) &&  (((cords.y+label_back->getLabelH())>y) && ((cords.y)<y))){
         lock=true;
         focused=true;
        
       
       }
    }
    const std::string getText(){

        return Text;
    }
    const int getInt(){

        return  std::stoi(Text);
    }
   
    void setVisible(bool b){

        visible=b;
    }

    bool isFocused(){

        return focused;
    }






};
// Drop down
class CDropDown{


    private:

    std::unique_ptr<CButton> CurrentSelectionButton;
    std::unique_ptr<Clabel> back_lable;
    std::vector<std::unique_ptr<CButton>> SelectionOptionsButtons;
    uint64_t CurrentSelection=0;
    Cordinates cords;
    SDL_Renderer * renderer;

    bool colapsed=true;
    int Xspace=50;

    std::vector<std::string> SelectionName;

    public:
    CDropDown(Cordinates c,SDL_Renderer* r,bool db, bool abs,bool v,TTF_Font * f,std::vector<std::string> &s,uint64_t ci){
        cords=c;
        renderer=r;

        SelectionName=s;
        CurrentSelection=ci;
        CurrentSelectionButton = std::make_unique<CButton>(SelectionName[CurrentSelection],r,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255});
         back_lable=std::make_unique<Clabel>(r,Cordinates{300,300},true,true,f);
         back_lable->setBackgroundColor(SDL_Color{0,0,0,255});

        for(uint64_t i=0;i<SelectionName.size();i++ ){

            SelectionOptionsButtons.push_back((std::make_unique<CButton>(SelectionName[i],r,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255})));
        }

        
    }

    void Render(int x,int y,int mx,int my,float dt,CCursor::cursorType &cursor){


         if(!colapsed) back_lable->Render(Cordinates{x,y},Cordinates{ SelectionOptionsButtons.back()->getX()-x+SelectionOptionsButtons.back()->getW(),SelectionOptionsButtons.back()->getY()-y+SelectionOptionsButtons.back()->getH()});
        CurrentSelectionButton->setText("Font: "+SelectionName[CurrentSelection]);      
        CurrentSelectionButton->UpdateText();  
        CurrentSelectionButton->Render(x,y);
        if(CurrentSelectionButton->CheckIfHover(mx,my,dt)) cursor =CCursor::Hand;


       
        if(!colapsed){
        SelectionOptionsButtons[0]->Render(x+Xspace,y+CurrentSelectionButton->getH());
        
        for(uint64_t i=1;i<SelectionOptionsButtons.size();i++ ){
            
             SelectionOptionsButtons[i]->Render(x+Xspace,SelectionOptionsButtons[i-1]->getY()+SelectionOptionsButtons[i-1]->getH());


        }

        if(CheckIfHoverOptions(mx, my,dt)) cursor =CCursor::Hand;

      }


    }

    void setCurrentSelection(uint64_t n){

        CurrentSelection=n;
    }
    bool CheckIfHoverOptions(int mx,int my,float dt){
        bool b=false;
         for(uint64_t i=0;i<SelectionOptionsButtons.size();i++ ){

            if(SelectionOptionsButtons[i]->CheckIfHover(mx, my, dt)) b=true;
         }
         return b;
    }

    void setCurrentSelectionByString (const std::string &s){

         for(uint64_t i=0;i<SelectionName.size();i++ ){

            if(DEBUG) std::cout<<"comp |"<<s<<"| to |"<<SelectionName[i]<<"|"<<std::endl;
            if(s==SelectionName[i]) {


                CurrentSelection=i;
                return;
            }

         }


    }

    const std::string getCurrentSelectionString (){

        return SelectionName[CurrentSelection];
    }

    int CheckIfClickedOption(int mx,int my){

        for(uint64_t i=0;i<SelectionOptionsButtons.size();i++ ){

            SelectionOptionsButtons[i]->setMouseLocation(mx,my);
            if(SelectionOptionsButtons[i]->CheckIfClicked()) return i;

        }

        return -1;
    }


    void CheckIfClickedCurrentSelection(int mx,int my){


        CurrentSelectionButton->setMouseLocation(mx,my);
        if(CurrentSelectionButton->CheckIfClicked()){

            colapsed=!colapsed;
        }
    }


    bool isColapsed(){

        return colapsed;
    }
};


