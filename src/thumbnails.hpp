#pragma once
#include "globals.hpp"
#include <SDL2/SDL_render.h>


class CThumbnail{
private:
    SDL_Texture * tex_thumb;
    bool loaded=false ;
    int ind;
    SDL_Color tavgcolor={0,0,0,255};
    Cordinates cords;




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
        if(!loaded) return;
        SDL_DestroyTexture(tex_thumb);
        std::cout<<"Destroyed thumbnail "<<ind<<std::endl;
    }
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


    
    SDL_Texture* getTexture(){


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
         std::vector<CThumbnail> thumbnails;
         int size;
         int thumbX;
         int thumbY;
         int currentIndex;
         int scrollOffset;
         int thumb_showing;
         SDL_Renderer* renderer;

    public:

        CThumbnailGroup(int amount,SDL_Renderer* vrenderer){
            renderer=vrenderer;
             for(int i=0; i<amount; i++){
                
                CThumbnail tthumb(renderer,i);
                thumbnails.push_back(tthumb);
             }

             size=thumbnails.size();
        //std::cout << "Created " <<size<<" thumbnais"<<std::endl;
        }


    
    CThumbnail& getThumbnailByInd(int ind){
        if(ind<0 || ind>=size){
             std::cout << "Invalid thumb ind  " <<ind<<std::endl;

        }

        return thumbnails[ind];
    }

    int getSize(){

        return  size;
    }


    void drawThumbnails(int &winW,int &winH){

        //int thumbX = INIT_THUMB_X-scrollOffset*(THUMB_PADDING+THUMB_WIDTH); // start padding
        //int thumbY = INIT_THUMB_Y;


         for (size_t i = 0; i < size; i++) {
            SDL_Rect rect = {thumbX, thumbY, THUMB_WIDTH, THUMB_HEIGHT};

            // highlight current image
            if (i == currentIndex) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // yellow border
                SDL_RenderDrawRect(renderer, &rect);
            }

            SDL_RenderCopy(renderer, thumbnails[i].getTexture(), NULL, &rect);

            thumbX += THUMB_WIDTH + THUMB_PADDING; // spacing

            thumbnails[i].setCords(thumbX, thumbY);

            if((thumbX<=winW) && (thumbX>=-THUMB_WIDTH/2)){

                thumb_showing+=1;
                
            }
        }
    }

    void drawBackground(){
         thumbX = INIT_THUMB_X-scrollOffset*(THUMB_PADDING+THUMB_WIDTH); // start padding
         thumbY = INIT_THUMB_Y;

        SDL_Rect bgThumBox;
        bgThumBox.x = thumbnails[0].getX()-INIT_THUMB_X/2-THUMB_WIDTH-THUMB_PADDING; // small padding
        bgThumBox.y = 0;
        bgThumBox.w = ((thumbnails.size()*(THUMB_WIDTH+INIT_THUMB_X))-INIT_THUMB_X)+(INIT_THUMB_X/2)*2;
        //bgThumBox.w = thumbnails[thumbnails.size()-1].getX()+THUMB_WIDTH;
        bgThumBox.h = THUMB_HEIGHT+INIT_THUMB_Y+thumbnails[0].getY();

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150); // black with 150/255 alpha
        SDL_RenderFillRect(renderer, &bgThumBox);




    }

    void drawSelection(){


        int thumbcurrentIndex=currentIndex-scrollOffset;
        SDL_Rect bgThumSel;
        //bgThumSel.x = (THUMB_PADDING+THUMB_WIDTH)*(thumbcurrentIndex)+INIT_THUMB_X/2; // sel start
        bgThumSel.x =  thumbnails[currentIndex].getX()-THUMB_WIDTH-THUMB_PADDING*2;
        bgThumSel.y = 0;
        bgThumSel.w = THUMB_WIDTH+THUMB_PADDING*2;
        bgThumSel.h = THUMB_HEIGHT+INIT_THUMB_Y+thumbY;


        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 150); // black with 150/255 alpha
        SDL_RenderFillRect(renderer, &bgThumSel);


    }


    void ReplaceThumbnailsAround(
        const std::vector<std::string> imageFiles
    ) {
        int around_size=thumb_showing*2 ;
       // std::cout << "Trying Replace Around "<<thumb_showing*2 <<"\n";
        for(int i=(currentIndex-around_size>0)?currentIndex-around_size:0; i<currentIndex+around_size;i++){
            //std::cout << "Trying Replace "<<i<<"\n";
            if(i<0 || i>thumbnails.size()-1) continue;

            thumbnails[i].LoadThumbnailImage(imageFiles[i],renderer);
            //ReplaceThumbnailWithImage(i,imageFiles[i],renderer,thumbnails,Loadedthumbnails);


        }

    }


    void Render(int &winH,int &winW){


        drawBackground();
        drawSelection();
        drawThumbnails(winW, winH);

    }

    void setCurrentIndex(int n){ currentIndex=n;}
    int  getCurrentIndex(){ return currentIndex;}   

    void setScrollOffset(int n){ scrollOffset=n;}
    int  getScrollOffset(){ return scrollOffset;}   
    
    void setThumbShowing(int n){ thumb_showing=n;}
    int  getThumbShowing(){ return thumb_showing;}    



    std::vector<CThumbnail> & getThumbnails(){


        return thumbnails;
    }


};