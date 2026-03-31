#pragma once
#include "globals.hpp"
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL2_rotozoom.h>
#include <SDL2/SDL_surface.h>
#include <condition_variable>
#include <SDL2/SDL2_imageFilter.h>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>



//images
class CImage{
    private:
        Cordinates cords={0,0};
        int angle=0;
        float zoom=1;
        int imgW,imgH;
        SDL_Texture *texture;
        SDL_Renderer *renderer;
        SDL_Surface* surf;
        float offsetX,offsetY;
        SDL_Color AvgColor;
        bool Loaded=false;
        int cload=0;
        std::atomic<bool> surfaceReady{false};
        std::atomic<bool> textureReady{false};
        std::mutex imageMutex;
        std::string CreationTime="";
        int WindowDecorationY=0;



        


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
        // sync loading
        SDL_Texture* loadImageFile(const std::string& path, SDL_Renderer* renderer) {
                if (surf) {
                    SDL_FreeSurface(surf);
                    surf = nullptr;  
                }
                

                if (texture) {
                    SDL_DestroyTexture(texture);
                    texture = nullptr; 
                }

                surf = IMG_Load(path.c_str());
                if (!surf) {
                    std::cout << "Failed to load: " << path << "\n";
                    return nullptr;
                }
                
                imgW = surf->w;
                imgH = surf->h;

                SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                return tex;  
            }

        SDL_Texture* rotatedTexture(int rot){

            if (!surf) return nullptr;
            SDL_DestroyTexture(texture);
            SDL_Surface* rotatedSurf = rotozoomSurface(surf, rot, 1.0, 1);
            int rw = rotatedSurf->w;
            int rh = rotatedSurf->h;

            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, rotatedSurf);

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
                        int W_comp=(winW-(imgW*zoom+offsetX))/2;
                        offsetX=W_comp;

                    }
                 


        }
        void SetDefaultTexture()
            {
                std::lock_guard<std::mutex> lock(imageMutex);

                if (surf) {
                    SDL_FreeSurface(surf);
                    surf = nullptr;
                }

                if (texture) {
                    SDL_DestroyTexture(texture);
                    texture = nullptr;
                }

                surf = SDL_CreateRGBSurfaceWithFormat(0, 100, 100, 32, SDL_PIXELFORMAT_RGBA32);
                if (!surf) {
                    std::cout << "Failed to create default surface: " << SDL_GetError() << std::endl;
                    return;
                }

                Uint32 gray = SDL_MapRGBA(surf->format, 128, 128, 128, 0);
                SDL_FillRect(surf, NULL, gray);

                imgW = 100;
                imgH = 100;

                texture = SDL_CreateTextureFromSurface(renderer, surf);
                if (!texture) {
                    std::cout << "Failed to create default texture: " << SDL_GetError() << std::endl;
                    return;
                }

                 if(DEBUG) std::cout << "Created default texture " <<std::endl;

                
            }

    std::string getFileTime(const fs::path& p)
    {
        auto ftime = fs::last_write_time(p);

        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now()
            + std::chrono::system_clock::now()
        );

        std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

        return std::string(std::ctime(&cftime));
    }

    public:



    
        CImage(SDL_Renderer *r){
           Loaded=false;
           if(DEBUG) std::cout << "Image constructor called"<<std::endl;
           zoom=1.0f;
           offsetX=0;
           offsetY=0;
           cords.x=0;
           cords.y=0;
           renderer=r;
           texture = nullptr;
           surf = nullptr;
           SetDefaultTexture();
           if(DEBUG) std::cout << "Image constructor exited"<<std::endl;



        }

        ~CImage(){
                if (surf) SDL_FreeSurface(surf);
                if (texture) SDL_DestroyTexture(texture);
                if(DEBUG) std::cout<<"Destroyed Image "<<std::endl;
            }
        CImage(const CImage&) = delete;
        CImage& operator=(const CImage&) = delete;

        void CenterImage(int winW,int winH){
            winH-=WindowDecorationY;
            zoom = std::min((float)winW / imgW, (float)winH / imgH);
                    offsetX = 0;
                    offsetY = THUMB_HEIGHT+INIT_THUMB_Y*2;

            calculate_offsets(zoom,winH,winW,imgH,imgW,offsetY,offsetX);

            cords.x=offsetX;
            cords.y=offsetY+WindowDecorationY;

        }

        void setWindowDecorationY(int y){
            WindowDecorationY=y;
        }
       void LoadImage_(const std::string& path,int winW,int winH){

            if (texture) {
                SDL_DestroyTexture(texture);
                texture = nullptr;
            }

    

            
            
    

            texture = loadImageFile(path, renderer);
           Loaded=true;
             CenterImage(winW,winH);
             
              
        }



        void calcZoom(int winW,int winH){
            
          

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

            }



        void Render(int winW,int winH){
            if(surfaceReady && !textureReady){
                
                CreateTextureFromSurface();
                CenterImage(winW, winH);
                
                

            }

        
            
            if (!texture) return;
            
           
            int renderW = int(imgW * zoom);
            int renderH = int(imgH * zoom);

           

            SDL_Rect dstRect;
          
                dstRect = { cords.x, cords.y, renderW, renderH };


            SDL_Point center = { dstRect.w / 2, dstRect.h / 2 };

            if (SDL_RenderCopyEx(renderer, texture, NULL, &dstRect, 0, &center, SDL_FLIP_NONE) != 0) {
                std::cout << "Render error: " << SDL_GetError() << std::endl;
            }


        }



        

        void Rotate90(){
            angle+=90;
            
          texture = rotatedTexture(angle);
            angle=angle%360;

        }

        void Rotate270(){
            angle+=270;
          texture = rotatedTexture(angle);

          angle=angle%360;

        }



        // async load 1 surf
        void LoadSurfaceOnly(const std::string& path,int wW,int wH)
            {
                std::lock_guard<std::mutex> lock(imageMutex);
                if(DEBUG) std::cout<<" Time: "<<  getFileTime(path)<<std::endl;
                CreationTime=getFileTime(path);
                
                if (!CreationTime.empty())
                CreationTime.pop_back();

                if (surf) {
                    SDL_FreeSurface(surf);
                    surf = nullptr;
                }

                SDL_Surface* loaded = IMG_Load(path.c_str());
                if (!loaded) return;

                surf = loaded;
                imgW = surf->w;
                imgH = surf->h;
              
                CenterImage(wW,wH);
                surfaceReady = true;
                   
            }

        // async load 2 surf->tex
        void CreateTextureFromSurface()
            {
                std::lock_guard<std::mutex> lock(imageMutex);

                if (!surfaceReady || textureReady) return;

                texture = SDL_CreateTextureFromSurface(renderer, surf);
                if (!texture) return;

                textureReady = true;
                Loaded = true;
                if(DEBUG) std::cout<<"texture Created"<<std::endl;
                 
            }

        // unused 
        // void Initialize(int winW,int winH,float &x,float& y,float& z){
        //     return;

     
        //     if(DEBUG)  std::cout<<"cord y"<<cords.y<<" zoom "<<zoom;
        //      x=cords.x;
        //      y=cords.y;
        //      z=zoom;
         
           
        // }    
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
        Cordinates getCords(){

            return cords;
        }

        const std::string getCreationTime()const {
  
            return CreationTime;
        }

        bool isLoaded(){return Loaded;}
        bool IsSurfaceReady() const { return surfaceReady; }
        bool IsTextureReady() const { return textureReady; }

        void setW(int n){imgW=n;}
        void setH(int n){imgH=n;}

        void setZoom(float n){zoom=n;}
         void setRotation(double n){angle=n;}
        void setCords(Cordinates c){
            if(Loaded)
            cords=c;
        
        }
        
        









};

//image group
class CImages{

    private:
    std::vector<std::unique_ptr<CImage>> images;
    std::vector<std::string> imageFiles;
    SDL_Renderer* renderer;
    int currentIndex;
    int size,winW,winH;
    std::thread loaderThread;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::queue<int> loadQueue;
    std::atomic<bool> running{true};
    std::deque<std::atomic<bool>> queued;
    
    public:
   


    CImages(SDL_Renderer* r,  std::vector<std::string> iF,int cind,int wW,int wH){
        renderer=r;
        imageFiles=iF;
        currentIndex=cind;
        size=imageFiles.size();
        winW=wW;
        winH=wH;
        for(int i=0; i<size; i++){
            images.push_back(std::make_unique<CImage>(renderer));
        }

        queued = std::deque<std::atomic<bool>>(size);
        for (int i = 0; i < size; i++) {
            queued[i] = false;
        }   

        loaderThread = std::thread([this]() {

            while (running)
            {
                int index = -1;
                
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    cv.wait(lock, [&] {
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
                        if(DEBUG)  std::cout<<"Loading async Surface "<<index<<" f size"<<imageFiles.size()<<std::endl;
                        images[index]->LoadSurfaceOnly(imageFiles[index],winW,winH);
                        if(DEBUG) std::cout<<"Loaded async Surface "<<index<<std::endl;
                    }
                }
            }
        });


        
        
        if(DEBUG) std::cout<<"Images constructor loaded "<<iF.size()<<std::endl;
    };

    void set_window(int wx,int wy){

        winW=wx;
        winH=wy;
    }
    //unused
    // void InitializeCurrentIndex(int winW,int winH, float& x, float&y,float& z){

    //         LoadAroundAsync(ASYNCLOADING);
    //         images[currentIndex]->Initialize(winW, winH,x,y,z);



    // }
    ~CImages()
    {
        running = false;
        cv.notify_all();
        if (loaderThread.joinable())
            loaderThread.join();
    }
    CImages(const CImages&) = delete;
    CImages& operator=(const CImages&) = delete;

    void LoadAround(int aroundnum,int winW,int winH){



        for(int i=-aroundnum;i<=aroundnum ;i++){

            int ind=0;

              if(i>0)
              ind=(currentIndex + i) % size;
              else if(i<0)
              ind = (currentIndex + i + size) % size;
              else
              ind=currentIndex;

           
            if(images[ind]->isLoaded()) continue;
            images[ind]->LoadImage_(imageFiles[ind],winW,winH);
            

        }


    }
    void LoadAroundAsync(int aroundnum)
    {
        if (size<=0) return;
        
        for (int i = -aroundnum; i <= aroundnum; i++)
        { 
          
            int ind = (currentIndex + i + size) % size;
           
            if(ind<0 || ind>=size) continue;
             
             
             
               
            if (!images[ind]->IsSurfaceReady() && !queued[ind].exchange(true))
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
                    
                    if (images[i]->isLoaded()|| images[i]->IsSurfaceReady()) {
               
                        images[i]->UnloadImage();
                        queued[i].exchange(false);
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
    void Render(int winW,int winH){
       
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


      void CenterCurrentImage(int winW, int winH){
        images[currentIndex]->CenterImage(winW, winH);

    }

    int NextImage(int i,int winW,int winH){
        if(DEBUG) std::cout<<"Next caled "<<std::endl;
        if(i>0)
        currentIndex=(currentIndex + i) % size;
        else
        currentIndex = (currentIndex + i + imageFiles.size()) % imageFiles.size();
       if(DEBUG) std::cout<<"Next by "<<i<<" ="<<currentIndex<<std::endl;
       LoadAroundAsync(ASYNCLOADING);
       UnLoadAround(UNLOADAT);
       images[currentIndex]->CenterImage(winW,winH);
       
    
          if(DEBUG) std::cout<<"Next ended "<<std::endl;
       return currentIndex;
        




    }

    void CurrentImageRotate90(int winW,int winH){


        images[currentIndex]->Rotate90();
        images[currentIndex]->CenterImage(winW,  winH);

        
    }


    void CurrentImageRotate270(int winW,int winH){


        images[currentIndex]->Rotate270();
        images[currentIndex]->CenterImage(winW,  winH);

    }

    void setCurrentImageCords(Cordinates c){
        images[currentIndex]->setCords(c);

        
    }

     float getCurrentImageZoom(){return images[currentIndex]->getZoom();}

     void setCurrentImageZoom(float z){

        images[currentIndex]->setZoom(z);
     }


    void moveCurrentImage(int dx,int dy){

        Cordinates init = images[currentIndex]->getCords();

        init.x+=dx;
        init.y+=dy;


        images[currentIndex]->setCords(init);

    }

    Cordinates getCurrentImageCords(){


        return  images[currentIndex]->getCords();
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

    SDL_Surface * getCurrentImageSurface(){


        return images[currentIndex]->getSurface();
    }

    const std::string getCurrentImageTime() const{

        return images[currentIndex]->getCreationTime();
    }

    bool IsSurfaceOfIndexReady(int i){

        return images[i]->IsSurfaceReady();

    }

    void addImage(const std::string path){
  
        imageFiles.push_back(path);
        queued.emplace_back(false);

        images.push_back(std::make_unique<CImage>(renderer));
        size++;


    }

    const bool getQueuedImage(int i){


        return queued[i].load();
    }

    void setCurrentImageWindowDecorationY(int y){

        images[currentIndex]->setWindowDecorationY(y);
    }

}; 