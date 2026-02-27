#pragma once
#include "globals.hpp"
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL2_rotozoom.h>
#include <cstddef>
#include <iterator>



class CImage{
    private:
        Cordinates cords;
        int angle=0;
        float zoom;
        int imgW,imgH;
        SDL_Texture *texture;
        SDL_Renderer *renderer;
        SDL_Surface* surf;
        float offsetX,offsetY;



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
                    if(DEBUG)std::cout<<"OFFSET Y: "<<offsetY<<std::endl;
                    if(DEBUG)std::cout<<"OFFSET X: "<<offsetX<<std::endl;
                    if(DEBUG)std::cout<<"zoom: "<<zoom<<std::endl;
                    if(DEBUG)std::cout<<"imgW: "<<imgW<<std::endl;
                    if(DEBUG)std::cout<<"imgH: "<<imgH<<std::endl;



        }

    public:
        CImage(SDL_Renderer *r){
           std::cout << "Image constructor called"<<std::endl;
           zoom=1.0f;
           offsetX=0;
           offsetY=0;
           renderer=r;
           texture = nullptr;
           std::cout << "Image constructor exited"<<std::endl;



        }

        ~CImage(){
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(texture);
            
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
            std::cout<<"Centered"<<std::endl;

        }
       void LoadImage(const std::string& path,int winW,int winH){

            if (texture) {
                SDL_DestroyTexture(texture);
                texture = nullptr;
            }

    

                
            
    

            texture = loadImageFile(path, renderer);
           
             CenterImage(winW,winH);
        }


        void UnloadImage(){
            

             SDL_FreeSurface(surf);
              SDL_DestroyTexture(texture);

        }



        void Render(){
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

        void setW(int n){imgW=n;}
        void setH(int n){imgH=n;}

        void setZoom(float n){zoom=n;}
         void setRotation(double n){angle=n;}
        void setCords(Cordinates c){cords=c;}
        
        









};


