#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_ttf.h>


#include <cstddef>
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#define DEBUG true

const int THUMB_WIDTH = 100;
const int THUMB_HEIGHT = 75;
const int INIT_THUMB_X = 10;
const int INIT_THUMB_Y = 10;
const int THUMB_PADDING = 10;

namespace fs = std::filesystem;








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



 SDL_Texture* loadImage(const std::string& path, SDL_Renderer* renderer, int& w, int& h) {
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        std::cout << "Failed to load: " << path << "\n";
        return nullptr;
    }
    w = surf->w;
    h = surf->h;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
    }


int main(int argc, char* argv[]) {
    std::vector<std::string> imageFiles;
    std::vector<SDL_Texture*> thumbnails;

    if (argc < 2) {
        std::cout << "Usage: viewer <image_path>\n";
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    if (TTF_Init() == -1) {
    std::cout << "TTF Init Error: " << TTF_GetError() << "\n";
    return 1;
}

// image load f

    fs::path firstImagePath(argv[1]);
    fs::path dir = firstImagePath.parent_path(); 


    // Supported extensions
    std::vector<std::string> exts = {".png", ".jpg", ".jpeg", ".bmp"};

    for (auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (std::find(exts.begin(), exts.end(), ext) != exts.end()) {
            printf("Loaded image ");

            if(DEBUG)std::cout<<"Loaded image "<<entry.path().string()<<std::endl;



            //debug_menu.print_dbg("\nLoaded image ");
            imageFiles.push_back(entry.path().string());
        }
    }


    std::sort(imageFiles.begin(), imageFiles.end());


    int currentIndex = 0;

    int thumbcurrentIndex=0;
    for (size_t i = 0; i < imageFiles.size(); i++) {
    if (imageFiles[i] == firstImagePath.string()) {
        currentIndex = i;
        break;
        }
    }


    SDL_Window* window = SDL_CreateWindow(
        "Image Viewer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1000, 700,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );





    ////prew
    for (const auto& imgPath : imageFiles) {
        int w, h;
        SDL_Texture* thumbTex = loadImage(imgPath, renderer, w, h);

        if (!thumbTex) continue;

        // create a scaled texture by copying to a render target texture
        SDL_Texture* scaledThumb = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            THUMB_WIDTH,
            THUMB_HEIGHT
        );

        // save current render target
        SDL_Texture* oldTarget = SDL_GetRenderTarget(renderer);

        // set new target and scale draw
        SDL_SetRenderTarget(renderer, scaledThumb);
        SDL_RenderClear(renderer);

        SDL_Rect dest{0,0,THUMB_WIDTH,THUMB_HEIGHT};
        SDL_RenderCopy(renderer, thumbTex, NULL, &dest);

        // restore old render target
        SDL_SetRenderTarget(renderer, oldTarget);

        SDL_DestroyTexture(thumbTex); // destroy original texture
        thumbnails.push_back(scaledThumb);
    }





    ///////img

 /*   SDL_Surface* surface = IMG_Load(argv[1]);
    if (!surface) {
        std::cout << "Failed to load image\n";
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    int imgW = surface->w;
    int imgH = surface->h;
    SDL_FreeSurface(surface);*/
    int imgW = 100;
    int imgH= 100;
   
    int thumbScroll=0;
    SDL_Texture* texture = loadImage(imageFiles[currentIndex], renderer, imgW, imgH);
    bool running = true;
    bool fullscreen = false;
    bool dragging = false;

    int winW, winH;
    SDL_GetWindowSize(window, &winW, &winH);

    // ---- Initial zoom to fit window (aspect ratio kept)
    float zoom = std::min((float)winW / imgW, (float)winH / imgH);

    float minZoom = zoom * 0.1f;
    float maxZoom = zoom * 20.0f;

    float offsetX = 0;
    float offsetY = 0;

    int lastMouseX = 0;
    int lastMouseY = 0;

    SDL_Event event;

    //font

    TTF_Font* font = TTF_OpenFont("./fonts/SFUIDisplay-Light.ttf", 18);
    if (!font) {
        std::cout << "Failed to load font: " << TTF_GetError() << "\n";
        return 1;
    }


    //main loop
    while (running) {
        //int image_x=0;
       // int image_y=0;
          // ---- Get window size
        SDL_GetWindowSize(window, &winW, &winH);

        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT)
                running = false;

            
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    int newWidth = event.window.data1;
                    int newHeight = event.window.data2;

                    std::cout << "Window resized to: "
                              << newWidth << " x "
                              << newHeight << std::endl;
                    zoom = std::min((float)winW / imgW, (float)winH / imgH);
                    offsetX = 0;
                    offsetY = THUMB_HEIGHT+INIT_THUMB_Y*2;
                    calculate_offsets(zoom,winH,winW,imgH,imgW,offsetY,offsetX);
                }
            if (event.window.event == SDL_WINDOWEVENT_MAXIMIZED || event.window.event == SDL_WINDOWEVENT_RESTORED)
                {
                    
                    std::cout << "Window was maximized!" << std::endl;
                   
                }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    running = false;

                if (event.key.keysym.sym == SDLK_f) {
                    fullscreen = !fullscreen;
                    SDL_SetWindowFullscreen(
                        window,
                        fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
                    );
                }
                  if (event.key.keysym.sym == SDLK_RIGHT) {
                    currentIndex = (currentIndex + 1) % imageFiles.size();
                    SDL_DestroyTexture(texture);
                    texture = loadImage(imageFiles[currentIndex], renderer, imgW, imgH);
                    // reset zoom & offsets if desired
                    zoom = std::min((float)winW / imgW, (float)winH / imgH);
                    offsetX = 0;
                    offsetY = THUMB_HEIGHT+INIT_THUMB_Y*2;
                    calculate_offsets(zoom,winH,winW,imgH,imgW,offsetY,offsetX);
                    

                }
                if (event.key.keysym.sym == SDLK_LEFT) {
                    currentIndex = (currentIndex - 1 + imageFiles.size()) % imageFiles.size();
                    SDL_DestroyTexture(texture);
                    texture = loadImage(imageFiles[currentIndex], renderer, imgW, imgH);
                    zoom = std::min((float)winW / imgW, (float)winH / imgH);
                    offsetX = 0;
                    offsetY = THUMB_HEIGHT+INIT_THUMB_Y*2;
                    calculate_offsets(zoom,winH,winW,imgH,imgW,offsetY,offsetX);
                }
                if (event.key.keysym.sym == SDLK_UP) {
                   thumbScroll+=1;
                   //currentIndex+=1;
                    //std::cout << "tmb" << std::endl;
                }
                if (event.key.keysym.sym == SDLK_DOWN) {
                   thumbScroll-=1;
                   // std::cout << "tmb" << std::endl;
                }
            }


            // ---- Mouse wheel zoom centered to cursor
            if (event.type == SDL_MOUSEWHEEL) {

                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                float oldZoom = zoom;

                if (event.wheel.y > 0)
                    zoom *= 1.1f;
                else if (event.wheel.y < 0)
                    zoom /= 1.1f;

                zoom = std::clamp(zoom, minZoom, maxZoom);

                float scaleChange = zoom / oldZoom;

                // Adjust offset so zoom happens toward mouse
                offsetX = mouseX - scaleChange * (mouseX - offsetX);
                offsetY = mouseY - scaleChange * (mouseY - offsetY);
            }
           /* if (event.type == SDL_MULTIGESTURE)
            {
                float zoomDelta = event.mgesture.dDist;

                if (zoomDelta > 0)
                    std::cout << "Zooming in\n";
                else if (zoomDelta < 0)
                    std::cout << "Zooming out\n";
            }*/

            // ---- Start dragging
            if (event.type == SDL_MOUSEBUTTONDOWN &&
                event.button.button == SDL_BUTTON_LEFT) {

                dragging = true;
                lastMouseX = event.button.x;
                lastMouseY = event.button.y;
            }

            // ---- Stop dragging
            if (event.type == SDL_MOUSEBUTTONUP &&
                event.button.button == SDL_BUTTON_LEFT) {
                dragging = false;
            }

            // ---- Drag motion
            if (event.type == SDL_MOUSEMOTION && dragging) {
                int dx = event.motion.x - lastMouseX;
                int dy = event.motion.y - lastMouseY;

                offsetX += dx;
                offsetY += dy;

                lastMouseX = event.motion.x;
                lastMouseY = event.motion.y;
            }
        }

      
        // ---- Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // optional background color
        SDL_RenderClear(renderer);

        // ---- Render image
        SDL_Rect dest;
        dest.w = imgW * zoom;
        dest.h = imgH * zoom;
        //dest.x = offsetX;
        //dest.y = offsetY;
        dest.x = offsetX;
        dest.y = offsetY;

       /* //h align
        if(dest.h+dest.y>winH){

        int H_comp=dest.h+dest.y-winH;
        dest.y-=H_comp;
        //if(DEBUG)std::cout<<"over: "<<W_comp<<std::endl;
    
        }*/

        //dest.x=winW-dest.w;
        

       // if(DEBUG)std::cout<<"dest.h: "<<dest.h+dest.y<<"winH"<<winH<<std::endl;
        SDL_RenderCopy(renderer, texture, NULL, &dest);

        // ---- Render thumbnails at top
        int thumbX = INIT_THUMB_X-thumbScroll*(THUMB_PADDING+THUMB_WIDTH); // start padding
        int thumbY = INIT_THUMB_Y;
        
        //thumbbg
        SDL_Rect bgThumBox;
        bgThumBox.x = thumbX-INIT_THUMB_X/2; // small padding
        bgThumBox.y = 0;
        bgThumBox.w = ((thumbnails.size()*(THUMB_WIDTH+INIT_THUMB_X))-INIT_THUMB_X)+(INIT_THUMB_X/2)*2;
        bgThumBox.h = THUMB_HEIGHT+INIT_THUMB_Y+thumbY;

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150); // black with 150/255 alpha
        SDL_RenderFillRect(renderer, &bgThumBox);


        //thumsel
         thumbcurrentIndex=currentIndex-thumbScroll;
        SDL_Rect bgThumSel;
        bgThumSel.x = (THUMB_PADDING+THUMB_WIDTH)*(thumbcurrentIndex)+INIT_THUMB_X/2; // sel start
        bgThumSel.y = 0;
        bgThumSel.w = THUMB_WIDTH+INIT_THUMB_X;
        bgThumSel.h = THUMB_HEIGHT+INIT_THUMB_Y+thumbY;

        if(bgThumSel.x+bgThumSel.w>winW){


            if(DEBUG)std::cout<<"thumb over"<<std::endl;
       //     thumbcurrentIndex=1;
        }else{

//            thumbcurrentIndex=currentIndex;
        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 150); // black with 150/255 alpha
        SDL_RenderFillRect(renderer, &bgThumSel);



        for (size_t i = 0; i < thumbnails.size(); i++) {
            SDL_Rect rect = {thumbX, thumbY, THUMB_WIDTH, THUMB_HEIGHT};

            // highlight current image
            if (i == currentIndex) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // yellow border
                SDL_RenderDrawRect(renderer, &rect);
            }

            SDL_RenderCopy(renderer, thumbnails[i], NULL, &rect);

            thumbX += THUMB_WIDTH + THUMB_PADDING; // spacing
        }



        



        // ---- Render info text (with black background)
        std::string info = "File: " + std::string(imageFiles[currentIndex]) +
                        "  Size: " + std::to_string(imgW) + "x" + std::to_string(imgH) +
                        "  Zoom: " + std::to_string((int)(zoom*100)) + "%";

        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, info.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        SDL_Rect textRect;
        textRect.x = 10;
        textRect.y = winH - textSurface->h - 10;
        textRect.w = textSurface->w;
        textRect.h = textSurface->h;

        // ---- Draw black semi-transparent rectangle behind text
        SDL_Rect bgRect = textRect;
        bgRect.x -= 5; // small padding
        bgRect.y -= 5;
        bgRect.w += 10;
        bgRect.h += 10;

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150); // black with 150/255 alpha
        SDL_RenderFillRect(renderer, &bgRect);

        // ---- Render text on top
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);


        //debug render

        std::string infot ="Beta version";

        SDL_Color textColorDbg = {255, 255, 255, 255};
        SDL_Surface* textSurfaceDbg = TTF_RenderText_Blended(font, infot.c_str(), textColorDbg);
        SDL_Texture* textTextureDbg = SDL_CreateTextureFromSurface(renderer, textSurfaceDbg);

        SDL_Rect textRectDbg;
        textRectDbg.x = 200;
        textRectDbg.y = 200;
        textRectDbg.w = textSurfaceDbg->w;
        textRectDbg.h = textSurfaceDbg->h;
        //textRectDbg.w = 300;
        //textRectDbg.h = 300;

        SDL_FreeSurface(textSurfaceDbg);
       
        SDL_RenderCopy(renderer, textTextureDbg, NULL,&textRectDbg);


        // ---- Present everything
        SDL_RenderPresent(renderer);

        SDL_DestroyTexture(textTextureDbg);
        SDL_DestroyTexture(textTexture);
    }


    //clean
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    TTF_CloseFont(font);
    TTF_Quit(); 

    return 0;
}