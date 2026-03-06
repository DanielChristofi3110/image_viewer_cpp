#pragma once
#include "globals.hpp"
#include "image.hpp"
#include "GUI.hpp"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"


class CThumbnail{
private:
    SDL_Texture * tex_thumb;
    SDL_Surface * surface;
    bool loaded=false ;
    int ind;
    SDL_Color tavgcolor={0,0,0,255};
    Cordinates cords;
    std::atomic<bool> loading{false};
    std::atomic<bool> ready{false};

    std::vector<unsigned char> pendingPixels;
    int pendingW = 0;
    int pendingH = 0;

    std::mutex pixelMutex;



    SDL_Color GetAverageColor(SDL_Surface* surface)
        {
            Uint64 totalR = 0;
            Uint64 totalG = 0;
            Uint64 totalB = 0;

            int pixelCount = surface->w * surface->h;

            SDL_LockSurface(surface);

            Uint8* pixels = (Uint8*)surface->pixels;
            int bpp = surface->format->BytesPerPixel;

            for (int y = 0; y < surface->h; y++)
            {
                for (int x = 0; x < surface->w; x++)
                {
                   Uint8* p = pixels + y * surface->pitch + x * 4;

                    totalR += p[0];
                    totalG += p[1];
                    totalB += p[2];
                }
            }

            SDL_UnlockSurface(surface);

            SDL_Color avg;
            avg.r = totalR / pixelCount;
            avg.g = totalG / pixelCount;
            avg.b = totalB / pixelCount;
            avg.a = 255;

            return avg;
        }
        SDL_Texture* loadThumbnailImageFile(
            const std::string& path,
            SDL_Renderer* renderer,
            int& outW,
            int& outH,
            int targetW,
            int targetH)
        {
            int width, height, channels;

            // Force RGBA (4 channels)
            unsigned char* data =
                stbi_load(path.c_str(), &width, &height, &channels, 4);

            if (!data)
            {
                std::cout << "Failed to load image: " << path << "\n";
                return nullptr;
            }

            unsigned char* resizedData =
                new unsigned char[targetW * targetH * 4];

            // ✅ Correct resize2 call
            bool success = stbir_resize_uint8_srgb(
                data,
                width,
                height,
                width * 4,          // input stride
                resizedData,
                targetW,
                targetH,
                targetW * 4,        // output stride
                STBIR_RGBA          // pixel layout only
            );

            if (!success)
            {
                std::cout << "Failed to resize image\n";
                stbi_image_free(data);
                delete[] resizedData;
                return nullptr;
            }

            SDL_Surface* surface =
                SDL_CreateRGBSurfaceWithFormatFrom(
                    resizedData,
                    targetW,
                    targetH,
                    32,
                    targetW * 4,
                    SDL_PIXELFORMAT_RGBA32);

            if (!surface)
            {
                stbi_image_free(data);
                delete[] resizedData;
                return nullptr;
            }

            outW = targetW;
            outH = targetH;

            tavgcolor = GetAverageColor(surface);

            SDL_Texture* texture =
                SDL_CreateTextureFromSurface(renderer, surface);

            SDL_FreeSurface(surface);
            stbi_image_free(data);
            delete[] resizedData;

            return texture;
        }



        void BackgroundLoad(const std::string& path, int targetW, int targetH)
        {
            int width, height, channels;

            unsigned char* data =
                stbi_load(path.c_str(), &width, &height, &channels, 4);

            if (!data)
            {
                loading = false;
                return;
            }

            std::vector<unsigned char> resized(targetW * targetH * 4);

            unsigned char* result =
                stbir_resize_uint8_srgb(
                    data,
                    width,
                    height,
                    width * 4,
                    resized.data(),
                    targetW,
                    targetH,
                    targetW * 4,
                    STBIR_RGBA
                );

            stbi_image_free(data);

            if (!result)
            {
                loading = false;
                return;
            }

            {
                std::lock_guard<std::mutex> lock(pixelMutex);
                pendingPixels = std::move(resized);
                pendingW = targetW;
                pendingH = targetH;
                ready = true;
            }

            loading = false;
        }
public:
   
        void loadThumbnailImageFromSurface(SDL_Renderer* renderer) {

            if(!surface) {
                std::cout<<"no surface"<<std::endl;
                return;}
            tavgcolor = GetAverageColor(surface);

            SDL_Texture* original = SDL_CreateTextureFromSurface(renderer, surface);
            if(!original) return;

            SDL_Texture* scaledThumb = SDL_CreateTexture(
                renderer,
                SDL_PIXELFORMAT_RGBA8888,
                SDL_TEXTUREACCESS_TARGET,
                THUMB_WIDTH,
                THUMB_HEIGHT
            );

            if (!scaledThumb) {
                SDL_DestroyTexture(original);
                return;
            }

            SDL_Texture* oldTarget = SDL_GetRenderTarget(renderer);
            SDL_SetRenderTarget(renderer, scaledThumb);

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);

            SDL_Rect dest{0, 0, THUMB_WIDTH, THUMB_HEIGHT};
            SDL_RenderCopy(renderer, original, NULL, &dest);

            SDL_SetRenderTarget(renderer, oldTarget);

            SDL_DestroyTexture(original);
            SDL_DestroyTexture(tex_thumb);

            tex_thumb = scaledThumb;
            loaded = true;
        }
    CThumbnail(SDL_Renderer* renderer,int i){
        ind=i;
         //std::cout << "Thumb constructor called for "<<ind<<std::endl;

        // Create empty texture for thumbnail
        SDL_Texture* scaledThumb = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            THUMB_WIDTH,
            THUMB_HEIGHT
        );
        

        if (!scaledThumb) {
            std::cout << "Failed to create thumbnail texture\n";
            
        }

        // Save current render target
        SDL_Texture* oldTarget = SDL_GetRenderTarget(renderer);

        // Set new target
        SDL_SetRenderTarget(renderer, scaledThumb);

        // Set draw color to yellow
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);

        // Clear texture with yellow
        SDL_RenderClear(renderer);

        // Restore previous render target
        SDL_SetRenderTarget(renderer, oldTarget);

        //thumbnails.push_back(scaledThumb);
       // Loadedthumbnails.push_back(false);
        tex_thumb=scaledThumb;
        //thumb_proc_ind++;
        loaded=false;

      // std::cout << "Thumb constructor exited for:" <<ind<<std::endl;



    }
    ~CThumbnail(){
        if(tex_thumb){
        SDL_DestroyTexture(tex_thumb);
        
        tex_thumb=nullptr;
    }
        std::cout<<"Destroyed thumbnail "<<ind<<std::endl;
    }

    CThumbnail(const CThumbnail&) = delete;
    CThumbnail& operator=(const CThumbnail&) = delete;


    void LoadThumbnailImage(const std::string& imgPath,SDL_Renderer* renderer) {

        if(THUMBNAIL_ASYNCLOADING){
          if (loaded || loading)
            return;

         std::cout<<"Loading tumb"<<std::endl;    
        loading = true;

        std::thread([this, imgPath]() {
                BackgroundLoad(imgPath, THUMB_WIDTH, THUMB_HEIGHT);
            }).detach();

        }else{
        //std::cout << "LoadThumbnailImage Call for " <<ind<<" loaded :"<<loaded<<std::endl;
        /* if (index >= thumbnails.size() || Loadedthumbnails[index]){
                std::cout<<"Skiped: "<<index<<std::endl;
                return;
            }*/
            //old
            
            if (loaded){
                //std::cout<<"Skiped: "<<ind<<std::endl;
                return;
            }
             
            int w, h;
            SDL_Texture* original = loadThumbnailImageFile(imgPath, renderer, w, h,100,100);
            std::cout << "LoadThumbnailImage Call for " <<ind<<" loaded :"<<loaded<<std::endl;
            // SDL_Texture* original = loadThumbnailImageFromSurface(renderer)
            if (!original)
                return;

            
            SDL_Texture* scaledThumb = SDL_CreateTexture(
                renderer,
                SDL_PIXELFORMAT_RGBA8888,
                SDL_TEXTUREACCESS_TARGET,
                THUMB_WIDTH,
                THUMB_HEIGHT
            );

            if (!scaledThumb) {
                SDL_DestroyTexture(original);
                return;
            }

        
            SDL_Texture* oldTarget = SDL_GetRenderTarget(renderer);

            
            SDL_SetRenderTarget(renderer, scaledThumb);

        
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);

            
            SDL_Rect dest{0, 0, THUMB_WIDTH, THUMB_HEIGHT};
            SDL_RenderCopy(renderer, original, NULL, &dest);

            
            SDL_SetRenderTarget(renderer, oldTarget);

            SDL_DestroyTexture(original);


            SDL_DestroyTexture(tex_thumb);

            
            tex_thumb = scaledThumb;
           loaded=true;
            // std::cout << "LoadThumbnailImage exit for " <<ind<<std::endl;
            
        }
             
        }

    void Update(SDL_Renderer* renderer)
        {
            if (!ready)
                return;
            std::cout<<"Updateing tumb "<<pendingPixels.size()<<std::endl;
            std::lock_guard<std::mutex> lock(pixelMutex);

            SDL_Surface* surface =
                SDL_CreateRGBSurfaceWithFormatFrom(
                    pendingPixels.data(),
                    pendingW,
                    pendingH,
                    32,
                    pendingW * 4,
                    SDL_PIXELFORMAT_RGBA32);

            if (!surface)
                return;

            tavgcolor = GetAverageColor(surface);

            SDL_Texture* newTexture =
                SDL_CreateTextureFromSurface(renderer, surface);

            SDL_FreeSurface(surface);

            if (newTexture)
            {
                SDL_DestroyTexture(tex_thumb);
                tex_thumb = newTexture;
                loaded = true;
            }
            pendingPixels.clear();
            //pendingPixels.shrink_to_fit();
            ready = false;
        }
    void UnloadLoad(){

        if(!FIX_WINDOWS){

            return;
        }

        loaded=false;

    }
    
    void setSurface(SDL_Surface * s){

        surface =s;
    }




    SDL_Texture * getTexture()const{


        return tex_thumb;
    }

    bool isLoaded(){


        return loaded;
    }

    int getInd(){

        return ind;
    }

    int getX(){

        return cords.x;
    }


    int getY(){

        return cords.y;
    }

    Cordinates& gerCords(){

        return cords;
    }

    void setCords(int x,int y){

        cords.x=x;
        cords.y=y;


    }
    SDL_Color getTavgcolor(){

        return tavgcolor;
    }

};



class CThumbnailGroup{
    private:
         std::vector<std::unique_ptr<CThumbnail>> thumbnails;
         std::vector<std::unique_ptr<CButton>> buttons;
         std::vector<std::string> imageFiles;
         int size;
         int thumbX;
         int thumbY;
         int currentIndex=0;
         int scrollOffset=0;
         int thumb_showing;
         Cordinates CindCords{0,0},FCords{0,0},LCords{0,0};
         SDL_Renderer* renderer;
         CImages* Images;
         bool visible=true;

    public:

        CThumbnailGroup(int amount,SDL_Renderer* vrenderer,CImages* im,TTF_Font *f,bool drawLabels,const std::vector<std::string>& files){
            renderer=vrenderer;
            imageFiles=files;
             for(int i=0; i<amount; i++){
                
               
               
                thumbnails.push_back(std::make_unique<CThumbnail>(renderer,i));
                if(drawLabels){


                //labels.push_back(std::make_unique<Clabel>(renderer,thumbnails[i]->gerCords(),false,false,f));
                buttons.push_back(std::make_unique<CButton>(" ",renderer,Cordinates{0,0},true,true,true,f,SDL_Color{255,255,255,255}));
                buttons.back()->setnColor({0,0,0,0});
                buttons.back()->sethColor({255,255,255,128});  
            }
             }

             size=thumbnails.size();
             Images=im;
       
        }

    CThumbnailGroup(const CThumbnailGroup&) = delete;
    CThumbnailGroup& operator=(const CThumbnailGroup&) = delete;


    
    CThumbnail& getThumbnailByInd(int ind){
        if(ind<0 || ind>=size){
             std::cout << "Invalid thumb ind  " <<ind<<std::endl;

        }

        return *thumbnails[ind];
    }

    int getSize(){

        return  size;
    }


    void drawThumbnails(int &winW,int &winH){
        thumb_showing=0;


         for (size_t i = 0; i < size; i++) {
            SDL_Rect rect = {thumbX, thumbY, THUMB_WIDTH, THUMB_HEIGHT};

            // highlight current image
            if (i == currentIndex) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // yellow border
                SDL_RenderDrawRect(renderer, &rect);
            }

            SDL_RenderCopy(renderer, thumbnails[i]->getTexture(), NULL, &rect);

            thumbX += THUMB_WIDTH + THUMB_PADDING; // spacing

            thumbnails[i]->setCords(thumbX, thumbY);

            if((thumbX<=winW) && (thumbX>=-THUMB_WIDTH/2)){

                thumb_showing+=1;
                
            }
        }

        FCords =thumbnails[0]->gerCords();
        LCords =thumbnails.back()->gerCords();
        //int gx=thumbnails[0]->getX();
       // std::cout<<gx<<std::endl;
    }

    void drawLabels(){
        if(buttons.empty()) return;
        for(int i=0; i<size; i++){

            Cordinates cordtemp=thumbnails[i]->gerCords();
            //cordtemp.x-=THUMB_WIDTH;
            //labels[i]->Render({cordtemp.x,cordtemp.y+20},std::to_string(i+1)+"/"+std::to_string(size));
            //buttons[i]->setText(std::to_string(i+1)+"/"+std::to_string(size));
            buttons[i]->Render(cordtemp.x-THUMB_WIDTH-THUMB_PADDING,cordtemp.y,THUMB_WIDTH,THUMB_HEIGHT);

        }


    }

    void CheckThumbnailPress(float dt,int mx,int my){
         if(buttons.empty()) return;

         for(int i=0;i<buttons.size();i++){
            //int x=buttons[i]->getX();
            //int y=buttons[i]->getY();
            buttons[i]->CheckIfHover(mx,my,dt);

         }


    }
      int CheckIfThumbnaiClicked(int s,int e,int mx,int my){
         if(buttons.empty()) return -1;

         for(int i=s;i<(e<0?buttons.size():e);i++){
            //int x=buttons[i]->getX();
            //int y=buttons[i]->getY();
            buttons[i]->setMouseLocation(mx, my);
            if(buttons[i]->CheckIfClicked()){


                std::cout<<"--Clicked thumbnail "<<i<<std::endl;
                currentIndex=i;
                return i;
            }

         }

         return -1;
    }

    void drawBackground(){
         thumbX = INIT_THUMB_X-scrollOffset*(THUMB_PADDING+THUMB_WIDTH); // start padding
         thumbY = INIT_THUMB_Y;

        SDL_Rect bgThumBox;
        bgThumBox.x = thumbnails[0]->getX()-INIT_THUMB_X/2-THUMB_WIDTH-THUMB_PADDING; // small padding
        bgThumBox.y = 0;

        bgThumBox.w = ((thumbnails.size()*(THUMB_WIDTH+INIT_THUMB_X))-INIT_THUMB_X)+(INIT_THUMB_X/2)*2;
        //bgThumBox.w = thumbnails[thumbnails.size()-1].getX()+THUMB_WIDTH;
        bgThumBox.h = THUMB_HEIGHT+INIT_THUMB_Y+thumbnails[0]->getY();

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150); // black with 150/255 alpha
        SDL_RenderFillRect(renderer, &bgThumBox);




    }

    void drawSelection(){


        int thumbcurrentIndex=currentIndex-scrollOffset;
        SDL_Rect bgThumSel;
        //bgThumSel.x = (THUMB_PADDING+THUMB_WIDTH)*(thumbcurrentIndex)+INIT_THUMB_X/2; // sel start
        bgThumSel.x =  thumbnails[currentIndex]->getX()-THUMB_WIDTH-THUMB_PADDING*2;
        bgThumSel.y = 0;
        CindCords.x=bgThumSel.x;
        CindCords.y=bgThumSel.y;

        if( bgThumSel.x<0){
           // scrollOffset--;

        }

        bgThumSel.w = THUMB_WIDTH+THUMB_PADDING*2;
        bgThumSel.h = THUMB_HEIGHT+INIT_THUMB_Y+thumbY;


        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 150); // black with 150/255 alpha
        SDL_RenderFillRect(renderer, &bgThumSel);


    }


    bool ReplaceThumbnailsAround() {
        //return false;
        bool all_loaded=true;
        int around_size=thumb_showing*2 ;
       std::cout << "Trying Replace Around "<<thumb_showing*2 <<"  "<<currentIndex<<"\n";
        for(int i=(currentIndex-around_size>0)?currentIndex-around_size:0; i<currentIndex+around_size;i++){
            //std::cout << "Trying Replace "<<i<<"\n";
            if(i<0 || i>thumbnails.size()-1) continue;

            if(THUMBNAIL_ASYNCLOADING){
            thumbnails[i]->LoadThumbnailImage(imageFiles[i],renderer);
            }else{
            //ReplaceThumbnailWithImage(i,imageFiles[i],renderer,thumbnails,Loadedthumbnails);
            
            if(!Images->IsSurfaceOfIndexReady(i)) all_loaded=false;
            if(!Images->IsSurfaceOfIndexReady(i) || thumbnails[i]->isLoaded())continue;
            thumbnails[i]->setSurface(Images->getSurfaceByIndex(i));
            thumbnails[i]->loadThumbnailImageFromSurface(renderer);
            
            }
        }
        return true;
    }


    bool ReplaceThumbnailsAround(int ind
    ) {
        //return false;
        bool all_loaded=true;
        int around_size=thumb_showing*2 ;
       std::cout << "Trying Replace Around "<<thumb_showing*2 <<"  "<<ind<<"\n";
        for(int i=(ind-around_size>0)?ind-around_size:0; i<ind+around_size;i++){
            //std::cout << "Trying Replace "<<i<<"\n";
            if(i<0 || i>thumbnails.size()-1) continue;

            if(THUMBNAIL_ASYNCLOADING){
            thumbnails[i]->LoadThumbnailImage(imageFiles[i],renderer);
            }else{
            //ReplaceThumbnailWithImage(i,imageFiles[i],renderer,thumbnails,Loadedthumbnails);
            
            if(!Images->IsSurfaceOfIndexReady(i)) all_loaded=false;
            if(!Images->IsSurfaceOfIndexReady(i) || thumbnails[i]->isLoaded())continue;
            thumbnails[i]->setSurface(Images->getSurfaceByIndex(i));
            thumbnails[i]->loadThumbnailImageFromSurface(renderer);
            
            }
        }
        return true;
    }


    void Render(int &winH,int &winW,int mx,int my,float dt){
        if(!visible) return;
        if(THUMBNAIL_ASYNCLOADING)
        for(int i=0; i<thumbnails.size(); i++){
        thumbnails[i]->Update(renderer);
    
      }
        drawBackground();
        drawSelection();
        drawThumbnails(winW, winH);
        drawLabels();
        CheckThumbnailPress(dt,mx,my);

    }

    void setCurrentIndex(int n){ currentIndex=n;}
    int  getCurrentIndex(){ return currentIndex;}   

    void setScrollOffset(int n){ scrollOffset=n;}
    int  getScrollOffset(){ return scrollOffset;}   
    
    void setThumbShowing(int n){ thumb_showing=n;}
    int  getThumbShowing(){ return thumb_showing;}    

    void UpdateScrollOffset(int n,int wW,int wH){
        if(LCords.x-THUMB_WIDTH<wW && n>0)
            return;

        if(FCords.x>THUMB_WIDTH && n<0)
            return;
        
        //currentIndex+=n;
        scrollOffset+=n;
        ReplaceThumbnailsAround(scrollOffset);
        
    
    
    }

    void MoveScrollTo(int n,int wW,int wH){
       int est_showing=0;
       int t_thumbX=0;


         for (size_t i = 0; i < size; i++) {


            t_thumbX += THUMB_WIDTH + THUMB_PADDING; // spacing



            if((t_thumbX<=wW) && (t_thumbX>=-THUMB_WIDTH/2)){

                est_showing+=1;
                
            }
        }

        if(n>size-est_showing){

            scrollOffset=size-est_showing;
        }else if(n<est_showing){


            scrollOffset=0;
        }else{


            scrollOffset=n;
        }

      


    }

    void NextThumbnail(int n,int wW,int wH){

        

         currentIndex+=n;

         if(currentIndex<0) {
            currentIndex=size-1;
             UpdateScrollOffset(size-thumb_showing,wW,wH);
        
        }
        if(currentIndex>=size){

            UpdateScrollOffset(-(size-thumb_showing)-1,wW,wH);

        }
         currentIndex=currentIndex%size;


         if(CindCords.x>wW-THUMB_WIDTH*2 && n>0){
                //scrollOffset+=n;
                UpdateScrollOffset(n,wW,wH);
         }


          if(CindCords.x<THUMB_WIDTH*2 && n<0){
                UpdateScrollOffset(n,wW,wH);
         }

    }



    const std::vector<std::unique_ptr<CThumbnail>>& getThumbnails() const {
    return thumbnails;
    }


    void UlnoanLoad(){

        for(int i=0 ;i<size; i++){
            thumbnails[i]->UnloadLoad();


        }

    }

    void setVisibility(bool b){visible=b;
        
        if(buttons.empty()) return;
        for(int i=0;i<buttons.size();i++){

            buttons[i]->setEnabled(b);
            //buttons[i].se
        }
    }
    bool getVisibility(){return visible;}


};