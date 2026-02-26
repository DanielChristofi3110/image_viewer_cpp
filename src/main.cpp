
#include "globals.hpp"
#include "thumbnails.hpp"
#include <SDL2/SDL_render.h>
#include <iostream>








//ofset calculation
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

//background



 SDL_Texture* CreateYellowBox(SDL_Renderer* renderer, int w, int h){
    // Create empty texture (render target)
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        w,
        h
    );

    if (!texture) return nullptr;

    // Enable rendering to texture
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    SDL_SetRenderTarget(renderer, texture);

    // Fill with yellow
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow
    SDL_RenderClear(renderer);

    // Reset render target back to screen
    SDL_SetRenderTarget(renderer, NULL);

    return texture;
}

int countLoadedThumbnails(std::vector<CThumbnail> cthu){

    int count=0;
    for(CThumbnail lt :cthu){
        if(lt.isLoaded())
        count++;

    }

    return count;

}
//text renderer
void RenderText(SDL_Renderer* renderer,
                     TTF_Font* font,
                     const std::string& text,
                     int x,
                     int y,
                     SDL_Color textColor,
                     bool drawBackground,bool absoluteCordinates,int &Nexty)
{
  
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), textColor);
    if (!textSurface) return;


    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture)
    {
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect textRect;
    textRect.x = x;
    textRect.y =absoluteCordinates?y:y-textSurface->h; // abs cord
    textRect.w = textSurface->w;
    textRect.h = textSurface->h;

    Nexty=textRect.y+textRect.h+10;
    SDL_FreeSurface(textSurface);

    
    if (drawBackground)
    {
        SDL_Rect bgRect = textRect;
        bgRect.x -= 5;
        bgRect.y -= 5;
        bgRect.w += 10;
        bgRect.h += 10;

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        SDL_RenderFillRect(renderer, &bgRect);
    }

   
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    
    SDL_DestroyTexture(textTexture);
}    



void ReplaceThumbnailsAround(
    int around_size,
    int index,
    const std::vector<std::string> imageFiles,
    SDL_Renderer* renderer,
    CThumbnailGroup & thumbnails
) {
     std::cout << "Trying Replace Around "<<index-around_size <<"\n";
    for(int i=(index-around_size>0)?index-around_size:0; i<index+around_size;i++){
        std::cout << "Trying Replace "<<i<<"\n";
        if(i<0 || i>thumbnails.getSize()-1) continue;

        thumbnails.getThumbnailByInd(i).LoadThumbnailImage(imageFiles[i],renderer);
        //ReplaceThumbnailWithImage(i,imageFiles[i],renderer,thumbnails,Loadedthumbnails);


    }

}


int main(int argc, char* argv[]) {
    std::vector<std::string> imageFiles;
    std::vector<SDL_Texture*> thumbnails;
    std::vector<bool> Loadedthumbnails;


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
            //printf("Loaded image ");

            if(DEBUG)std::cout<<"Loaded image "<<entry.path().string()<<std::endl;



            //debug_menu.print_dbg("\nLoaded image ");
            imageFiles.push_back(entry.path().string());
        }
    }

    std::cout<<"------------------Images Loaded-----------------------"<<std::endl; 
    std::sort(imageFiles.begin(), imageFiles.end());
     std::cout<<"------------------Images Sorted-----------------------"<<std::endl; 

    int currentIndex = 0;

    int thumbcurrentIndex=0;
    for (size_t i = 0; i < imageFiles.size(); i++) {
    if (imageFiles[i] == firstImagePath.string()) {
        currentIndex = i;
        break;
        }
    }

    //AA
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    SDL_Window* window = SDL_CreateWindow(
        "Image Viewer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1000, 700,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    );
    std::cout<<"------------------Created window-----------------------"<<std::endl; 

    



    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );




    std::cout<<"------------------Created  render-----------------------"<<std::endl; 


      //font

    TTF_Font* font = TTF_OpenFont("./fonts/SFUIDisplay-Light.ttf", 18);
    if (!font) {
        std::cout << "Failed to load font: " << TTF_GetError() << "\n";
        return 1;
    }

    std::cout<<"------------------Loaded font-----------------------"<<std::endl; 
    int thumb_proc_ind = 0;
    const int imageFiles_size = imageFiles.size();

     CThumbnailGroup thumbgroup(imageFiles.size(),renderer);
    std::cout<<"------------------Loaded Thumbnails-----------------------"<<std::endl; 
    
  




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
    SDL_Texture* texture = loadImage(imageFiles[currentIndex], renderer, imgW, imgH,gAvgColor,false);
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

  
    SDL_Texture* backgroundTexture = CreateRadialGradientTexture(renderer, winW, winH,gAvgColor);

    //main loop
    while (running) {
        auto start = std::chrono::high_resolution_clock::now();
        //int image_x=0;
       // int image_y=0;

       int thumb_showing=0;
          // ---- Get window size
        SDL_GetWindowSize(window, &winW, &winH);


      
        // ---- Clear screen
        //gAvgColor.g=250;
        //std::cout<<"-------------------AVG: "<< int(thumbgroup.getThumbnailByInd(currentIndex).getTavgcolor().b)<<"current index: "<<currentIndex<<std::endl;
        if(true){
        SDL_SetRenderDrawColor(renderer, int(thumbgroup.getThumbnailByInd(currentIndex).getTavgcolor().r), int(thumbgroup.getThumbnailByInd(currentIndex).getTavgcolor().g), int(thumbgroup.getThumbnailByInd(currentIndex).getTavgcolor().b), 255); // optional background color
        SDL_RenderClear(renderer);
        
        
        }else {
            SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
        }
        // renderBacground(renderer,winW,winH);
         //SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
       


        // ---- Render image

        //ReplaceThumbnailWithImage(currentIndex,imageFiles[currentIndex], renderer, thumbnails,Loadedthumbnails);
       
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
        //int thumbX = INIT_THUMB_X-thumbScroll*(THUMB_PADDING+THUMB_WIDTH); // start padding
        //int thumbY = INIT_THUMB_Y;
        
        thumbgroup.setCurrentIndex(currentIndex);
        thumbgroup.setScrollOffset(thumbScroll);
        thumbgroup.setThumbShowing(thumb_showing);
        thumbgroup.Render(winH, winW);
        thumb_showing=thumbgroup.getThumbShowing();


       

        



        // ---- Render info text (with black background)
        std::string info = "File: " + std::string(imageFiles[currentIndex]) +
                        "  Size: " + std::to_string(imgW) + "x" + std::to_string(imgH) +
                        "  Zoom: " + std::to_string((int)(zoom*100)) + "%";

        //SDL_Color info_color = {255, 255, 255, 255};
        
        {
            int tempytext;
            RenderText(renderer,
                            font,
                            info,
                            0,
                            winH,
                            {255, 255, 255, 255},
                            true,
                        false,
                    tempytext);
            }
      
       // std::string debugText = "Free mode"+std::to_string(winH);
       //std::string debugText = "Free mode";
       
        if (free_mode)
        {
            int tempytext;
            RenderText(renderer,
                            font,
                            "Free mode",
                            THUMB_PADDING,
                            THUMB_WIDTH,
                            {255, 255, 255, 255},
                            true,
                        true,
                        tempytext);
        }
        if(debug_mode){ 
            int tempytext;
             
            
            RenderText(renderer,
                            font,
                            "Fps:"+std::to_string(fps),
                            winW-100,
                            THUMB_WIDTH,
                            {255, 0, 0, 255},
                            true,
                        true,
                    tempytext);

            int lthu=countLoadedThumbnails(thumbgroup.getThumbnails());
            RenderText(renderer,
                            font,
                            "Preloaded thumbnails:"+std::to_string(lthu)+" "+std::to_string((lthu * 100) / imageFiles_size) + "%",
                            winW-250,
                            tempytext,
                            {255, 0, 0, 255},
                            true,
                        true,
                    tempytext);
                    
                    
                    
                    
                    }

        // ---- Present everything
        SDL_RenderPresent(renderer);

        //dest
        //SDL_DestroyTexture(textTextureDbg);
        //SDL_DestroyTexture(textTexture);
        //SDL_FreeSurface(textSurface);  //mem leak
        

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
                    std::cout<<"thumb showing "<<thumb_showing<<std::endl;
                    calculate_offsets(zoom,winH,winW,imgH,imgW,offsetY,offsetX);
                    thumbgroup.setThumbShowing(thumb_showing);
                    thumbgroup.ReplaceThumbnailsAround(imageFiles);
                   // SDL_DestroyTexture(backgroundTexture);
                    //backgroundTexture = CreateRadialGradientTexture(renderer, newWidth, newHeight,thumbgroup.getThumbnailByInd(currentIndex).getTavgcolor());
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
                  if (event.key.keysym.sym == SDLK_SPACE) {
                    free_mode=!free_mode;
                     std::cout<<"free: "<<free_mode<<std::endl;


                  }
                  if (event.key.keysym.sym == SDLK_RIGHT) {
                     Loadthumbnails=true;
                    currentIndex = (currentIndex + 1) % imageFiles.size();
                    SDL_DestroyTexture(texture);
                    texture = loadImage(imageFiles[currentIndex], renderer, imgW, imgH,gAvgColor,false);
                    // reset zoom & offsets if desired
                    zoom = std::min((float)winW / imgW, (float)winH / imgH);
                    offsetX = 0;
                    offsetY = THUMB_HEIGHT+INIT_THUMB_Y*2;
                    calculate_offsets(zoom,winH,winW,imgH,imgW,offsetY,offsetX);
                if (!free_mode){
                    if(currentIndex-thumbScroll>thumb_showing-3){
                            std::cout<<"next "<<std::endl;
                            thumbScroll+=1;

                    }
                    if((currentIndex-thumbScroll<1) &&(currentIndex>2)){
                            std::cout<<"back "<<std::endl;
                            thumbScroll-=2;
                          

                    }else if (currentIndex<2) {
                        thumbScroll=0;
                    }else if (currentIndex>imageFiles.size()-2) {
                         thumbScroll=imageFiles.size()-2;
                    }
                }
                    

                }
                if (event.key.keysym.sym == SDLK_LEFT) {
                    Loadthumbnails=true;
                    currentIndex = (currentIndex - 1 + imageFiles.size()) % imageFiles.size();
                    SDL_DestroyTexture(texture);
                    texture = loadImage(imageFiles[currentIndex], renderer, imgW, imgH,gAvgColor,false);
                    zoom = std::min((float)winW / imgW, (float)winH / imgH);
                    offsetX = 0;
                    offsetY = THUMB_HEIGHT+INIT_THUMB_Y*2;
                    calculate_offsets(zoom,winH,winW,imgH,imgW,offsetY,offsetX);
                if (!free_mode){
                    if(currentIndex-thumbScroll>thumb_showing-3){
                            std::cout<<"next "<<std::endl;
                            thumbScroll+=1;

                    }
                    if((currentIndex-thumbScroll<1) &&((currentIndex>2))){
                            std::cout<<"back "<<std::endl;
                            thumbScroll-=2;
                            

                    }else if (currentIndex<2) {
                        thumbScroll=0;
                    }else if (currentIndex>imageFiles.size()-2) {
                         thumbScroll=imageFiles.size()-2;
                    }
                }
                }
                if (event.key.keysym.sym == SDLK_UP) {
                     //ReplaceThumbnailsAround(thumb_showing*2, thumbScroll, imageFiles, renderer, thumbnails, Loadedthumbnails);
                    thumbgroup.setCurrentIndex(thumbScroll);
                      thumbgroup.setThumbShowing(thumb_showing);
                     thumbgroup.ReplaceThumbnailsAround(imageFiles);

                    if((thumbScroll>=0) &&(thumbScroll<=imageFiles.size())) thumbScroll+=1;
                    if(thumbScroll<0) thumbScroll=0; 
                    if(thumbScroll>=imageFiles.size()) thumbScroll=imageFiles.size()-1; 
                   
                }
                if (event.key.keysym.sym == SDLK_DOWN) {
                   // ReplaceThumbnailsAround(thumb_showing*2, thumbScroll, imageFiles, renderer, thumbnails, Loadedthumbnails);
                     thumbgroup.setCurrentIndex(thumbScroll);
                     thumbgroup.setThumbShowing(thumb_showing);
                   thumbgroup.ReplaceThumbnailsAround(imageFiles);
                   if((thumbScroll>=0) &&(thumbScroll<=imageFiles.size())) thumbScroll-=1;
                    if(thumbScroll<0) thumbScroll=0;
                    if(thumbScroll>=imageFiles.size()) thumbScroll=imageFiles.size()-1; 
                     
                    
                }

                 if (event.key.keysym.sym == SDLK_d) {
                  
                    if(DEBUG) debug_mode=!debug_mode;
                     
                    
                }
                    std::cout << "tmb " <<currentIndex-thumbScroll <<" scroll:"<<thumbScroll<<" ind:"<<currentIndex<<std::endl;
                    std::cout<<"winW over "<<thumb_showing<<std::endl;

                    
                    
            }
            if(Loadthumbnails){
               //ReplaceThumbnailsAround(thumb_showing*2, currentIndex, imageFiles, renderer,thumbgroup);
               thumbgroup.ReplaceThumbnailsAround(imageFiles); 
               //SDL_DestroyTexture(backgroundTexture);
              // backgroundTexture=CreateRadialGradientTexture(renderer, winW, winH,thumbgroup.getThumbnailByInd(currentIndex).getTavgcolor());
               Loadthumbnails=false;
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
        auto end = std::chrono::high_resolution_clock::now();

        // Calculate duration
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        fps=round((float)1000/(duration.count()/1000));
        //std::cout << "Execution time: " <<(float)1000/(duration.count()/1000) << " microseconds\n";

    }


    //clean
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    TTF_CloseFont(font);
    TTF_Quit(); 

    return 0;
}