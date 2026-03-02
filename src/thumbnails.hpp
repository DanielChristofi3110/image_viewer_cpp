#pragma once
#include "globals.hpp"
#include "image.hpp"
#include "GUI.hpp"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <string>


class CThumbnail{
private:
    SDL_Texture * tex_thumb;
    SDL_Surface * surface;
    bool loaded=false ;
    int ind;
    SDL_Color tavgcolor={0,0,0,255};
    Cordinates cords;



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
                    Uint8* p = pixels + y * surface->pitch + x * bpp;

                    Uint32 pixelValue;
                    memcpy(&pixelValue, p, bpp);

                    Uint8 r, g, b;
                    SDL_GetRGB(pixelValue, surface->format, &r, &g, &b);

                    totalR += r;
                    totalG += g;
                    totalB += b;
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
    SDL_Texture* loadThumbnailImageFile(const std::string& path, SDL_Renderer* renderer, int& w, int& h) {
            SDL_Surface* surf = IMG_Load(path.c_str());
            if (!surf) {
                std::cout << "Failed to load: " << path << "\n";
                return nullptr;
            }
            w = surf->w;
            h = surf->h;

            tavgcolor = GetAverageColor(surf);
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_FreeSurface(surf);
            return tex;
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
        //std::cout << "LoadThumbnailImage Call for " <<ind<<" loaded :"<<loaded<<std::endl;
        /* if (index >= thumbnails.size() || Loadedthumbnails[index]){
                std::cout<<"Skiped: "<<index<<std::endl;
                return;
            }*/
            if (loaded){
                //std::cout<<"Skiped: "<<ind<<std::endl;
                return;
            }
             
            int w, h;
            SDL_Texture* original = loadThumbnailImageFile(imgPath, renderer, w, h);
           // SDL_Texture* original = loadThumbnailImageFromSurface(renderer);
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


    void UnloadLoad(){

        if(!FIX_WINDOWS){

            return;
        }

        loaded=false;

    }
    
    void setSurface(SDL_Surface * s){

        surface =s;
    }




    SDL_Texture * getTexture(){


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
         std::vector<std::unique_ptr<Clabel>> labels;
         int size;
         int thumbX;
         int thumbY;
         int currentIndex;
         int scrollOffset;
         int thumb_showing;
         SDL_Renderer* renderer;
         CImages* Images;

    public:

        CThumbnailGroup(int amount,SDL_Renderer* vrenderer,CImages* im,TTF_Font *f,bool drawLabels){
            renderer=vrenderer;
             for(int i=0; i<amount; i++){
                
               
               
                thumbnails.push_back(std::make_unique<CThumbnail>(renderer,i));
                if(drawLabels)
                labels.push_back(std::make_unique<Clabel>(renderer,thumbnails[i]->gerCords(),false,false,f));
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

        //int gx=thumbnails[0]->getX();
       // std::cout<<gx<<std::endl;
    }

    void drawLabels(){
        if(labels.empty()) return;
        for(int i=0; i<size; i++){

            Cordinates cordtemp=thumbnails[i]->gerCords();
            cordtemp.x-=THUMB_WIDTH;
            labels[i]->Render({cordtemp.x,cordtemp.y+20},std::to_string(i+1)+"/"+std::to_string(size));

        }


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
        bgThumSel.w = THUMB_WIDTH+THUMB_PADDING*2;
        bgThumSel.h = THUMB_HEIGHT+INIT_THUMB_Y+thumbY;


        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 150); // black with 150/255 alpha
        SDL_RenderFillRect(renderer, &bgThumSel);


    }


    bool ReplaceThumbnailsAround(
    ) {
        //return false;
        bool all_loaded=true;
        int around_size=thumb_showing*2 ;
       // std::cout << "Trying Replace Around "<<thumb_showing*2 <<"\n";
        for(int i=(currentIndex-around_size>0)?currentIndex-around_size:0; i<currentIndex+around_size;i++){
            //std::cout << "Trying Replace "<<i<<"\n";
            if(i<0 || i>thumbnails.size()-1) continue;

            //thumbnails[i]->LoadThumbnailImage(imageFiles[i],renderer);
            //ReplaceThumbnailWithImage(i,imageFiles[i],renderer,thumbnails,Loadedthumbnails);

            if(!Images->IsSurfaceOfIndexReady(i)) all_loaded=false;
            if(!Images->IsSurfaceOfIndexReady(i) || thumbnails[i]->isLoaded())continue;
            thumbnails[i]->setSurface(Images->getSurfaceByIndex(i));
            thumbnails[i]->loadThumbnailImageFromSurface(renderer);


        }
        return true;
    }


    void Render(int &winH,int &winW){


        drawBackground();
        drawSelection();
        drawThumbnails(winW, winH);
        drawLabels();

    }

    void setCurrentIndex(int n){ currentIndex=n;}
    int  getCurrentIndex(){ return currentIndex;}   

    void setScrollOffset(int n){ scrollOffset=n;}
    int  getScrollOffset(){ return scrollOffset;}   
    
    void setThumbShowing(int n){ thumb_showing=n;}
    int  getThumbShowing(){ return thumb_showing;}    




    const std::vector<std::unique_ptr<CThumbnail>>& getThumbnails() const {
    return thumbnails;
    }


    void UlnoanLoad(){

        for(int i=0 ;i<size; i++){
            thumbnails[i]->UnloadLoad();


        }

    }


};