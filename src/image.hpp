#pragma once
#include "globals.hpp"
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL2_rotozoom.h>
#include <SDL2/SDL_surface.h>
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



class CImage{
    private:
        Cordinates cords={0,0};
        int angle=0;
        float zoom;
        int imgW,imgH;
        SDL_Texture *texture;
        SDL_Renderer *renderer;
        SDL_Surface* surf;
        float offsetX,offsetY;
        SDL_Color AvgColor;
        bool Loaded=false;
        std::atomic<bool> surfaceReady{false};
        std::atomic<bool> textureReady{false};
        std::mutex imageMutex;



        


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

        SDL_Texture* loadImageFile(const std::string& path, SDL_Renderer* renderer) {
            surf = IMG_Load(path.c_str());
            if (!surf) {
                std::cout << "Failed to load: " << path << "\n";
                return nullptr;
            }
            imgW = surf->w;
            imgH = surf->h;

           
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
           // SDL_FreeSurface(surf);
            return tex;
            }

        SDL_Texture* rotatedTexture(int rot){

            if (!surf) return nullptr;
            SDL_DestroyTexture(texture);
            // Rotate surface 90 degrees clockwise
            SDL_Surface* rotatedSurf = rotozoomSurface(surf, rot, 1.0, 1);
            int rw = rotatedSurf->w;
            int rh = rotatedSurf->h;

            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, rotatedSurf);

            //SDL_FreeSurface(surf);
            SDL_FreeSurface(rotatedSurf);
            imgH=rh;
            imgW=rw;

            return tex;


        }
        void calculate_offsets(float zoom , int winH,int winW,int imgH,int imgW,float &offsetY, float &offsetX){

                    if((imgH)*zoom+offsetY>winH){

                        int H_comp=imgH*zoom+offsetY-winH;
                        offsetY-=H_comp;
                    }
                    if(imgW*zoom+offsetX<winW){
                        //if(DEBUG)std::cout<<"dest.w+dest.y "<<dest.w+dest.x<<"winH"<<winW<<std::endl;
                        int W_comp=(winW-(imgW*zoom+offsetX))/2;
                        offsetX=W_comp;

                    }
                   // if(DEBUG)std::cout<<"OFFSET Y: "<<offsetY<<std::endl;
                    //if(DEBUG)std::cout<<"OFFSET X: "<<offsetX<<std::endl;
                    //if(DEBUG)std::cout<<"zoom: "<<zoom<<std::endl;
                    //if(DEBUG)std::cout<<"imgW: "<<imgW<<std::endl;
                    //if(DEBUG)std::cout<<"imgH: "<<imgH<<std::endl;



        }

    public:



    
        CImage(SDL_Renderer *r){
           Loaded=false;
           std::cout << "Image constructor called"<<std::endl;
           zoom=1.0f;
           offsetX=0;
           offsetY=0;
           renderer=r;
           texture = nullptr;
           surf = nullptr;
           std::cout << "Image constructor exited"<<std::endl;



        }

        ~CImage(){
                if (surf) SDL_FreeSurface(surf);
                if (texture) SDL_DestroyTexture(texture);
                std::cout<<"Destroyed Image "<<std::endl;
            }


        void CenterImage(int winW,int winH){
            //return;

            zoom = std::min((float)winW / imgW, (float)winH / imgH);
                    offsetX = 0;
                    offsetY = THUMB_HEIGHT+INIT_THUMB_Y*2;

            calculate_offsets(zoom,winH,winW,imgH,imgW,offsetY,offsetX);

            cords.x=offsetX;
            cords.y=offsetY;
            std::cout<<"Centered "<<cords.y<<std::endl;

        }
       void LoadImage(const std::string& path,int winW,int winH){

            if (texture) {
                SDL_DestroyTexture(texture);
                texture = nullptr;
            }

    

                
            
    

            texture = loadImageFile(path, renderer);
           Loaded=true;
             CenterImage(winW,winH);
              
        }

        void calcZoom(int winW,int winH){
            
            std::cout<<"zoomlevel "<<winW<<" "<<imgW<<std::endl;

            zoom = std::min((float)winW / imgW, (float)winH / imgH);
        }

        void UnloadImage(){
                if (surf) {
                    SDL_FreeSurface(surf);
                    surf = nullptr;      
                }

                if (texture) {
                    SDL_DestroyTexture(texture);
                    texture = nullptr;     
                }

                Loaded = false;
                textureReady=false;
                surfaceReady=false;
                std::cout<<"Unloaded Image "<<std::endl;
            }



        void Render(int winW,int winH){
            // std::cout<<textureReady<<" "<<zoom<<std::endl;
            if(surfaceReady && !textureReady){

                CreateTextureFromSurface();
            }
            
            if (!texture) return;
            
           
           // zoom=1;
            int renderW = int(imgW * zoom);
            int renderH = int(imgH * zoom);

           

            SDL_Rect dstRect;

                dstRect = { cords.x, cords.y, renderW, renderH };

                //std::cout<<dstRect.y<<" "<<cords.y<<std::endl;

            SDL_Point center = { dstRect.w / 2, dstRect.h / 2 };

            if (SDL_RenderCopyEx(renderer, texture, NULL, &dstRect, 0, &center, SDL_FLIP_NONE) != 0) {
                std::cout << "Render error: " << SDL_GetError() << std::endl;
            }
            //std::cout << "Rendering" << " " <<imgW<<" x "<<zoom<< std::endl;
        }



        

        void SyncZoomOffXOffY(float& z,float &x,float &y){

            z=zoom;
            x=cords.x;
            y=cords.y;





        }
        void Rotate90(){
            angle+=90;
            if(angle>=360) angle=0;
          texture = rotatedTexture(angle);

        }


        void LoadSurfaceOnly(const std::string& path)
            {
                std::lock_guard<std::mutex> lock(imageMutex);

                if (surf) {
                    SDL_FreeSurface(surf);
                    surf = nullptr;
                }

                SDL_Surface* loaded = IMG_Load(path.c_str());
                if (!loaded) return;

                surf = loaded;
                imgW = surf->w;
                imgH = surf->h;

                surfaceReady = true;
            }


        void CreateTextureFromSurface()
            {
                std::lock_guard<std::mutex> lock(imageMutex);

                if (!surfaceReady || textureReady) return;

                texture = SDL_CreateTextureFromSurface(renderer, surf);
                if (!texture) return;

                textureReady = true;
                Loaded = true;
                std::cout<<"texture Created"<<std::endl;
                // We no longer need the surface after texture creation
                //SDL_FreeSurface(surf);
                //surf = nullptr;
            }


        void Initialize(int winW,int winH,float &x,float& y,float& z){


            while (!surfaceReady) {
             
            }
            CreateTextureFromSurface();
             CenterImage(winW,winH);
             std::cout<<"cord y"<<cords.y<<" zoom "<<zoom;
             x=cords.x;
             y=cords.y;
             z=zoom;
            //calcZoom(winW,winH);
           
        }    
        SDL_Texture* getTexture(){return texture;}

        int getW(){

            return imgW;
        }

        int getH(){

            return imgH;
        }

        float getZoom(){

            return zoom;
        }

        int getRotation(){

            return angle;
        }

        SDL_Surface * getSurface(){

            return surf;

        }

        bool isLoaded(){return Loaded;}
        bool IsSurfaceReady() const { return surfaceReady; }
        bool IsTextureReady() const { return textureReady; }

        void setW(int n){imgW=n;}
        void setH(int n){imgH=n;}

        void setZoom(float n){zoom=n;}
         void setRotation(double n){angle=n;}
        void setCords(Cordinates c){cords=c;}
        
        









};


class CImages{

    private:
    std::vector<std::unique_ptr<CImage>> images;
    std::vector<std::string> imageFiles;
    SDL_Renderer* renderer;
    int currentIndex;
    int size;
    std::thread loaderThread;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::queue<int> loadQueue;
    std::atomic<bool> running{true};
    
    public:
   


    CImages(SDL_Renderer* r,  std::vector<std::string> iF,int cind,int winW,int winH){
        renderer=r;
        imageFiles=iF;
        currentIndex=cind;
        size=imageFiles.size();
        for(int i=0; i<size; i++){
            images.push_back(std::make_unique<CImage>(renderer));
        }

        loaderThread = std::thread([this]() {

            while (running)
            {
                int index = -1;
                
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    cv.wait(lock, [this] {
                        return !loadQueue.empty() || !running;
                    });

                    if (!running) return;

                    index = loadQueue.front();
                    loadQueue.pop();
                }

                if (index >= 0 && index < size)
                {
                    if (!images[index]->IsSurfaceReady())
                    {
                         std::cout<<"Loading async Surface "<<index<<std::endl;
                        images[index]->LoadSurfaceOnly(imageFiles[index]);
                    }
                }
            }
        });


        
        
        std::cout<<"Images constructor loaded "<<iF.size()<<std::endl;
    };


    void InitializeCurrentIndex(int winW,int winH, float& x, float&y,float& z){

            LoadAroundAsync(ASYNCLOADING);
            images[currentIndex]->Initialize(winW, winH,x,y,z);



    }
    ~CImages()
    {
        running = false;
        cv.notify_all();
        if (loaderThread.joinable())
            loaderThread.join();
    }

    void LoadAround(int aroundnum,int winW,int winH){

        //for(int i=0)

        for(int i=-aroundnum;i<=aroundnum ;i++){

            int ind=0;

              if(i>0)
              ind=(currentIndex + i) % size;
              else if(i<0)
              ind = (currentIndex + i + size) % size;
              else
              ind=currentIndex;

           
            if(images[ind]->isLoaded()) continue;
            images[ind]->LoadImage(imageFiles[ind],winW,winH);
            

        }


    }
    void LoadAroundAsync(int aroundnum)
    {
        for (int i = -aroundnum; i <= aroundnum; i++)
        {
            int ind = (currentIndex + i + size) % size;

            if (!images[ind]->IsSurfaceReady())
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                loadQueue.push(ind);
                cv.notify_one();
            }
        }
      

    }

    void UnLoadAround(int aroundnum){
            for(int i = 0; i < size; i++){

                int diff = abs(i - currentIndex);
                int circularDiff = std::min(diff, size - diff);

                if (circularDiff > aroundnum) {
                    if (images[i]->isLoaded()) {
                        images[i]->UnloadImage();
                    }
                }
            }
        }
    
        void UpdateTexturesFromSurfaces()
        {
            for (auto& img : images)
            {
                if (img->IsSurfaceReady() && !img->IsTextureReady())
                {
                    std::cout<<"Createing Texture"<<std::endl;
                    img->CreateTextureFromSurface();
                }
            }
        }
    void Render(int& cind,float zoom,float x,float y,int winW,int winH){
       // currentIndex=cind;
       cind=currentIndex;
       //UpdateTexturesFromSurfaces();
        //images[currentIndex]->CenterImage(winW, winH);
        images[currentIndex]->setZoom(zoom);
        images[currentIndex]->setCords({int(x),int(y)});
        images[currentIndex]->Render(winW,winH);



    }

    int getCurrentImageW(){

        return  images[currentIndex]->getW();
    }
    int getCurrentImageH(){

        return  images[currentIndex]->getH();
    }


    int getLoadedImages(){

        int cnt=0;
        for(int i=0; i<size;i++){
            if(images[i]->isLoaded())
                cnt++;
            
        }

        return cnt;
    }

    void SyncZoomOffXOffYofCurrentImage(float &z,float& x,float& y){
        images[currentIndex]->SyncZoomOffXOffY(z, x, y);

    }
      void CenterCurrentImage(int winW, int winH){
        images[currentIndex]->CenterImage(winW, winH);

    }

    int NextImage(int i,int winW,int winH,float &z,float &x,float& y){
        //i=0;
         std::cout<<"Next caled "<<std::endl;
      //  if(i+currentIndex<0 || i+currentIndex>=size) return;
        if(i>0)
        currentIndex=(currentIndex + i) % size;
        else
        currentIndex = (currentIndex + i + imageFiles.size()) % imageFiles.size();
        std::cout<<"Next by "<<i<<" ="<<currentIndex<<std::endl;
        //currentIndex=i+currentIndex;
      // LoadAround(1, winW, winH);
       LoadAroundAsync(ASYNCLOADING);
       UnLoadAround(UNLOADAT);
       images[currentIndex]->calcZoom(winW,winH);
       images[currentIndex]->CenterImage(winW,winH);
       float z2,x2,y2;
       images[currentIndex]->SyncZoomOffXOffY(z2, x2, y2);
       z=z2;
       x=x2;
       y=y2;

       return currentIndex;
        




    }

    void CurrentImageRotate90(int winW,int winH,float& z,float& x,float &y){


        images[currentIndex]->Rotate90();
        images[currentIndex]->CenterImage(winW,  winH);

        float z2,x2,y2;
        images[currentIndex]->SyncZoomOffXOffY(z2, x2, y2);
        z=z2;
        x=x2;
        y=y2;
    }


    int getCurrentIndex(){return currentIndex;}
    int getReadySurface(){
        int cnt=0;
        for(int i=0; i<size; i++)
        
        if(images[i]->IsSurfaceReady()){

            cnt++;
        }

        return cnt;
        }
    int getQueueSize(){return loadQueue.size();}

    int getCurrentImageRotation(){return images[currentIndex]->getRotation();}
    
    SDL_Surface * getSurfaceByIndex(int i){


        return images[i]->getSurface();
    }

    bool IsSurfaceOfIndexReady(int i){

        return images[i]->IsSurfaceReady();

    }

}; 