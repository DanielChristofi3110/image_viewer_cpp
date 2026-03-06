
#include "globals.hpp"
#include "thumbnails.hpp"
#include "image.hpp"
#include "GUI.hpp"
#include "background.hpp"
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <iostream>
#include <ostream>
#include <string>
#ifdef _WIN32
#include <dwmapi.h>
#endif







bool hide_ui=false;
float deltaTime=0;



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
    #ifdef _WIN32
    SetProcessDPIAware();
    #endif
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
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE| SDL_WINDOW_ALLOW_HIGHDPI
    );
    std::cout<<"------------------Created window-----------------------"<<std::endl;

    #ifdef _WIN32
    HWND hwnd = GetActiveWindow();
    BOOL dark = TRUE;

    // Windows 10 1809+
    DwmSetWindowAttribute(
        hwnd,
        20, // DWMWA_USE_IMMERSIVE_DARK_MODE
        &dark,
        sizeof(dark)
    );
    #endif



    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE
    );




    std::cout<<"------------------Created  render-----------------------"<<std::endl;


    //font

    TTF_Font* font = TTF_OpenFont((execDir_Windows+"/fonts/SFUIDisplay-Light.ttf").c_str(), 18);
    if (!font) {
        std::cout << "Failed to load font: " << TTF_GetError() << "\n";
        return 1;
    }

    std::cout<<"------------------Loaded font-----------------------"<<std::endl;
    int thumb_proc_ind = 0;
    const int imageFiles_size = imageFiles.size();


    std::cout<<"------------------Loaded Thumbnails-----------------------"<<std::endl;

    

    bool running = true;
    bool fullscreen = false;
    bool dragging = false;

    int winW, winH;
    
    SDL_GetRendererOutputSize(renderer, &winW, &winH);
  
    CImages Images(renderer,imageFiles,currentIndex,winW,winH);




 
    float zoom = 0;

    float offsetX = 0;
    float offsetY = 0;

    int lastMouseX = 0;
    int lastMouseY = 0;

   
    Images.LoadAroundAsync(ASYNCLOADING);
    CThumbnailGroup thumbgroup(imageFiles.size(),renderer,&Images,font,true,imageFiles);
    thumbgroup.ReplaceThumbnailsAround(currentIndex,winW/THUMB_WIDTH);
    thumbgroup.setCurrentIndex(currentIndex);
    thumbgroup.MoveScrollTo(currentIndex, winW, winH);


    Clabel RotateLeftLabel(renderer,{400,400},true,true,font);
    RotateLeftLabel.LoadSVGtoLabel((execDir_Windows+"/resources/vector/RotateLeft.svg").c_str(),0.03);
    RotateLeftLabel.setIconPositionLeft();

    CButton RotateLeftButton("R",renderer,{400,400},true,true,true,font,{255,255,255,255});
    RotateLeftButton.setSvgIcon((execDir_Windows+"/resources/vector/RotateLeft.svg").c_str(), true,0.03f);



    CButton RotateRightButton("Shift+R",renderer,{400,400},true,true,true,font,{255,255,255,255});
    RotateRightButton.setSvgIcon((execDir_Windows+"/resources/vector/RotateRight.svg").c_str(), true,0.03f);


    Clabel RotateRightLabel(renderer,{400,400},true,true,font);
    RotateRightLabel.LoadSVGtoLabel((execDir_Windows+"/resources/vector/RotateRight.svg").c_str(),0.03);
    RotateRightLabel.setIconPositionLeft();



    Clabel ResolutionLabel(renderer,{400,400},true,true,font);
    ResolutionLabel.LoadSVGtoLabel((execDir_Windows+"/resources/vector/Resolution.svg").c_str(),0.03);
    ResolutionLabel.setIconPositionLeft();

    Clabel ZoomLabel(renderer,{400,400},true,true,font);
    ZoomLabel.LoadSVGtoLabel((execDir_Windows+"/resources/vector/Zoom.svg").c_str(),0.03);
    ZoomLabel.setIconPositionLeft();

    Clabel FileLabel(renderer,{400,400},true,true,font);
    FileLabel.LoadSVGtoLabel((execDir_Windows+"/resources/vector/File.svg").c_str(),0.03);
    FileLabel.setIconPositionLeft();


    Clabel UnhideTipLabel(renderer,{400,400},true,true,font);


    Clabel InfoLabel(renderer,{400,400},true,false,font);
    CDebugLabels DebugLabel(renderer,{400,400},font);

    CButton NextImageRightButton("",renderer,{200,200},true,true,true,font,{64,255,64,255});
    NextImageRightButton.setSvgIcon((execDir_Windows+"/resources/vector/ArrowRight.svg").c_str(),false,0.06);

    CButton NextImageLeftButton("",renderer,{200,200},true,true,true,font,{64,255,64,255});
    NextImageLeftButton.setSvgIcon((execDir_Windows+"/resources/vector/ArrowLeft.svg").c_str(),false,0.06);


    SDL_Event event;

    CBackground background(renderer);
    //SDL_Texture* backgroundTexture = CreateRadialGradientTexture(renderer, winW, winH,{0,0,0,255});


    Uint32 lastTime = SDL_GetTicks();

     int thumb_showing=0;
    //main loop
    while (running) {
        auto start = std::chrono::high_resolution_clock::now();

        SDL_GetRendererOutputSize(renderer, &winW, &winH);



       
        background.StartLerp({thumbgroup.getThumbnailByInd(currentIndex).getTavgcolor().r,thumbgroup.getThumbnailByInd(currentIndex).getTavgcolor().g,thumbgroup.getThumbnailByInd(currentIndex).getTavgcolor().b,255}, 0.5f);
        background.Update(deltaTime);
        background.Render();



      

        Images.Render(winW,winH);
        currentIndex= Images.getCurrentIndex();
        zoom=Images.getCurrentImageZoom();
        Cordinates c=Images.getCurrentImageCords();
        offsetX=c.x;
        offsetY=c.y;

 

        
        thumb_showing=thumbgroup.getThumbShowing();








        // ---- Render info text (with black background)
        std::string info = "File: " + std::string(imageFiles[currentIndex]) +
        "  Size: " + std::to_string(Images.getCurrentImageW()) + "x" + std::to_string(Images.getCurrentImageH()) +
        "  Zoom: " + std::to_string((int)(zoom*100)) + "%";



        FileLabel.setVisibility(!hide_ui);
        ResolutionLabel.setVisibility(!hide_ui);
        ZoomLabel.setVisibility(!hide_ui);
        RotateRightLabel.setVisibility(!hide_ui);
        RotateLeftLabel.setVisibility(!hide_ui);
        RotateRightButton.setEnabled(!hide_ui);
        RotateLeftButton.setEnabled(!hide_ui);
        UnhideTipLabel.setVisibility(hide_ui);
        thumbgroup.setVisibility(!hide_ui);




        UnhideTipLabel.Render({0,winH-UnhideTipLabel.getLabelH()}, "Ctrl+H to unhide UI");
        std::string DisplayFilePath = imageFiles[currentIndex].substr(imageFiles[currentIndex].find_last_of((delim),imageFiles[currentIndex].length()));
        FileLabel.Render({0,winH-FileLabel.getLabelH()}, "File: " + DisplayFilePath.substr(1,DisplayFilePath.length()));
        ResolutionLabel.Render({0,FileLabel.getNexty()-FileLabel.getLabelH()*2}, "Size: "+std::to_string(Images.getCurrentImageW()) + "x" + std::to_string(Images.getCurrentImageH()));
        ZoomLabel.Render({0,ResolutionLabel.getNexty()-ResolutionLabel.getLabelH()*2}, "Zoom: " + std::to_string((int)(zoom*100)) + "%");



        int lthu=0;

        auto& tmg= thumbgroup.getThumbnails();
        for(auto& t :tmg){

            if(t->isLoaded()){lthu++;}
        }
        std::vector<std::string> strs;
        strs.push_back("Fps:"+std::to_string(fps));
        strs.push_back("Preloaded thumbnails:"+std::to_string(lthu)+" "+std::to_string((lthu * 100) / imageFiles_size) + "%");
        strs.push_back("Image Rotation "+std::to_string(Images.getCurrentImageRotation()));
        strs.push_back("Image Loaded "+std::to_string(Images.getLoadedImages()));
        strs.push_back("Current image "+std::to_string(Images.getCurrentIndex()));
        strs.push_back("Load Queue  "+std::to_string(Images.getQueueSize()));
        strs.push_back("Ready surfaces  "+std::to_string(Images.getReadySurface()));
        strs.push_back("Mouse (x,y) "+std::to_string(lastMouseX)+","+std::to_string(lastMouseY));
        strs.push_back(info);
        strs.push_back("DeltaT  "+std::to_string(deltaTime));
        DebugLabel.setVisibility(debug_mode);
        DebugLabel.setCords(0, THUMB_WIDTH);
        DebugLabel.Render(strs);




        NextImageRightButton.setEnabled(!hide_ui);
        NextImageLeftButton.setEnabled(!hide_ui);
        {

            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            thumbgroup.Render(winH, winW,mouseX,mouseY,deltaTime);
            NextImageRightButton.CheckIfHover(mouseX,mouseY,deltaTime);
            NextImageLeftButton.CheckIfHover(mouseX,mouseY,deltaTime);
            RotateLeftButton.CheckIfHover(mouseX,mouseY,deltaTime);
            RotateRightButton.CheckIfHover(mouseX,mouseY,deltaTime);

        }

        RotateRightButton.Render(0,ZoomLabel.getNexty()-ZoomLabel.getLabelH()*2);

        RotateLeftButton.Render(0,RotateRightButton.getY()-RotateRightButton.getH());
        NextImageRightButton.Render(winW/2,winH-NextImageRightButton.getH());
        NextImageLeftButton.Render(winW/2-NextImageLeftButton.getW(),winH-NextImageLeftButton.getH());

        SDL_RenderPresent(renderer);

        

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


                Images.CenterCurrentImage(winW,winH);
     

                //thumbgroup.setThumbShowing(thumb_showing);
                thumbgroup.ReplaceThumbnailsAround();
                thumbgroup.UlnoanLoad();

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
                    thumbgroup.UlnoanLoad();
                }
                if (event.key.keysym.sym == SDLK_r) {
                    Images.CurrentImageRotate90(winW,winH);
                }

                if (event.key.keysym.sym == SDLK_h &&event.key.keysym.sym & KMOD_CTRL) {
                    hide_ui=!hide_ui;
                }
                if (event.key.keysym.sym == SDLK_SPACE) {
                    //free_mode=!free_mode;
                    std::cout<<"free: "<<free_mode<<std::endl;


                }
                if (event.key.keysym.sym == SDLK_RIGHT) {
                    Loadthumbnails=true;
                    int ind=Images.NextImage(1,winW,winH);
                    thumbgroup.NextThumbnail(1,winW,winH);

                    


                }
                if (event.key.keysym.sym == SDLK_LEFT) {
                    Loadthumbnails=true;
                    int ind=Images.NextImage(-1,winW,winH);

                    thumbgroup.NextThumbnail(-1,winW,winH);
                    
                }
                if (event.key.keysym.sym == SDLK_UP) {
                  
        

                    

                    thumbgroup.UpdateScrollOffset(1,winW,winH);
                    thumbgroup.ReplaceThumbnailsAround();
      
                }
                if (event.key.keysym.sym == SDLK_DOWN) {

   

                

                    thumbgroup.UpdateScrollOffset(-1,winW,winH);
                    thumbgroup.ReplaceThumbnailsAround();



                }

                if (event.key.keysym.sym == SDLK_d) {

                    if(DEBUG) debug_mode=!debug_mode;


                }
                std::cout << "tmb " <<currentIndex-thumbgroup.getScrollOffset() <<" scroll:"<<thumbgroup.getScrollOffset()<<" ind:"<<currentIndex<<std::endl;
                std::cout<<"winW over "<<thumb_showing<<std::endl;



            }

            if(Loadthumbnails){

                thumbgroup.ReplaceThumbnailsAround();

                Loadthumbnails=false;
            }

        
            if (event.type == SDL_MOUSEWHEEL) {

                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);


                if(mouseY>THUMB_HEIGHT){


                    float oldZoom = Images.getCurrentImageZoom();
                    zoom=oldZoom;
                    if (event.wheel.y > 0)
                        zoom *= 1.1f;
                    else if (event.wheel.y < 0)
                        zoom /= 1.1f;

                

                    float scaleChange = zoom / oldZoom;

                
                
                    offsetX = mouseX - scaleChange * (mouseX - offsetX);
                    offsetY = mouseY - scaleChange * (mouseY - offsetY);

                    Images.setCurrentImageZoom(zoom);
                    Images.setCurrentImageCords({(int)offsetX,(int)offsetY});

                }else {

                    std::cout<<"THUMB MOUSE SCROLL "<<event.wheel.y <<std::endl;

                    
                    thumbgroup.UpdateScrollOffset(event.wheel.y,winH,winW);
                }

        
            }
          
            if (event.type == SDL_MOUSEBUTTONDOWN &&
                event.button.button == SDL_BUTTON_LEFT) {

                dragging = true;
            lastMouseX = event.button.x;
            lastMouseY = event.button.y;




      
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            NextImageRightButton.setMouseLocation(mouseX, mouseY);
            NextImageLeftButton.setMouseLocation(mouseX, mouseY);
            RotateLeftButton.setMouseLocation(mouseX,mouseY);
            RotateRightButton.setMouseLocation(mouseX,mouseY);
            int nimg= thumbgroup.CheckIfThumbnaiClicked(0, -1, mouseX, mouseY);

            if( RotateLeftButton.CheckIfClicked()){

                Images.CurrentImageRotate90(winW,winH);
            }

            if( RotateRightButton.CheckIfClicked()){

                Images.CurrentImageRotate270(winW,winH);
            }

            if(nimg!=-1){
                Loadthumbnails=true;
                int ind=Images.NextImage(nimg-currentIndex,winW,winH);
               

            }


            if(NextImageRightButton.CheckIfClicked()|| NextImageLeftButton.CheckIfClicked()){
                Loadthumbnails=true;
                int ind=Images.NextImage(NextImageRightButton.CheckIfClicked()?1:-1,winW,winH);
                thumbgroup.NextThumbnail(NextImageRightButton.CheckIfClicked()?1:-1,winW,winH);
              
            }


                }


                if (event.type == SDL_MOUSEBUTTONUP &&
                    event.button.button == SDL_BUTTON_LEFT) {
                    dragging = false;
                    }


                    if (event.type == SDL_MOUSEMOTION && dragging) {
                        int dx = event.motion.x - lastMouseX;
                        int dy = event.motion.y - lastMouseY;

                        
                        Images.moveCurrentImage(dx, dy);
                        lastMouseX = event.motion.x;
                        lastMouseY = event.motion.y;
                    }
        }
        auto end = std::chrono::high_resolution_clock::now();

  
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        fps=round((float)1000/(duration.count()/1000));

        Uint32 currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f; 
        lastTime = currentTime;

   

    }



    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    TTF_CloseFont(font);
    TTF_Quit();

    return 0;
}
