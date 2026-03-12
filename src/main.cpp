
#include "globals.hpp"
#include "thumbnails.hpp"
#include "image.hpp"
#include "GUI.hpp"
#include "background.hpp"
#include "FrameControl.hpp"
#include "FileScanner.hpp"
#include "Clipboard.hpp"
#include "Cursor.hpp"
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_set>

#ifdef _WIN32
#include <dwmapi.h>
#include<Windows.h>
#endif
#ifdef __linux__
#include <unistd.h>
#include <limits.h>
#endif





bool hide_ui=false;
float deltaTime=0;

 int DES_FPS = 30;
 bool windowActive=true;


//background

int estimateFrameDelat(int dfps){


    return 1000/dfps;
}


#ifdef _WIN32
std::string getExecutableDirectory() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string exePath(buffer);
    return exePath.substr(0, exePath.find_last_of("\\/")); // Get the directory
}

#ifdef _DEBUG
void EnableDebugConsole()
{
    AllocConsole();

    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    std::cout.clear();
    std::cerr.clear();
    std::cin.clear();

    std::cout << "Debug console enabled\n";
}
#endif

#endif

#ifdef __linux__
#include <unistd.h>
#include <limits.h>


std::string getExecutableDirectory() {
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len == -1) {
        return ""; // failed
    }
    buffer[len] = '\0';
    std::string exePath(buffer);
    return exePath.substr(0, exePath.find_last_of("/")); // get directory
}
#endif


int main(int argc, char* argv[]) {
    #ifdef _WIN32
    SetProcessDPIAware();
    #endif


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

    execDir=getExecutableDirectory();
    // image load f
    CFileScanner FileScanner(argv[1],1);

    // std::cout<<"------------------Images Loaded-----------------------"<<std::endl;
  
    // std::cout<<"------------------Images Sorted-----------------------"<<std::endl;

    int currentIndex = 0;

    int thumbcurrentIndex=0;

    currentIndex=FileScanner.getInitCurrentIndex();
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
    #ifdef _DEBUG
    EnableDebugConsole();
    #endif
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

    TTF_Font* font = TTF_OpenFont((execDir+"/fonts/SFUIDisplay-Light.ttf").c_str(), 18);
    if (!font) {
        std::cout << "Failed to load font: " << TTF_GetError() << "\n";
        return 1;
    }

    std::cout<<"------------------Loaded font-----------------------"<<std::endl;
    int thumb_proc_ind = 0;
    //const int imageFiles_size = imageFiles.size();


    std::cout<<"------------------Loaded Thumbnails-----------------------"<<std::endl;

    

    bool running = true;
    bool fullscreen = false;
    bool dragging = false;

    int winW, winH;
    
    SDL_GetRendererOutputSize(renderer, &winW, &winH);
  
    //CImages Images(renderer,imageFiles,currentIndex,winW,winH);
    std::shared_ptr<CImages> Images = std::make_shared<CImages>(renderer,FileScanner.getImageFiles(),currentIndex,winW,winH);



 
    float zoom = 0;

    float offsetX = 0;
    float offsetY = 0;

    int lastMouseX = 0;
    int lastMouseY = 0;

   
    Images->LoadAroundAsync(ASYNCLOADING);
    CThumbnailGroup thumbgroup(FileScanner.getImageFilesSize(),renderer,Images,font,true,FileScanner.getImageFiles());
    thumbgroup.ReplaceThumbnailsAround(currentIndex,winW/THUMB_WIDTH);
    thumbgroup.setCurrentIndex(currentIndex);
    thumbgroup.MoveScrollTo(currentIndex, winW, winH);


    Clabel RotateLeftLabel(renderer,{400,400},true,true,font);
    RotateLeftLabel.LoadSVGtoLabel((execDir+"/resources/vector/RotateLeft.svg").c_str(),0.03);
    RotateLeftLabel.setIconPositionLeft();

    CButton RotateLeftButton("R",renderer,{400,400},true,true,true,font,{255,255,255,255});
    RotateLeftButton.setSvgIcon((execDir+"/resources/vector/RotateLeft.svg").c_str(), true,0.03f);



    CButton RotateRightButton("Shift+R",renderer,{400,400},true,true,true,font,{255,255,255,255});
    RotateRightButton.setSvgIcon((execDir+"/resources/vector/RotateRight.svg").c_str(), true,0.03f);


    Clabel RotateRightLabel(renderer,{400,400},true,true,font);
    RotateRightLabel.LoadSVGtoLabel((execDir+"/resources/vector/RotateRight.svg").c_str(),0.03);
    RotateRightLabel.setIconPositionLeft();



    Clabel ResolutionLabel(renderer,{400,400},true,true,font);
    ResolutionLabel.LoadSVGtoLabel((execDir+"/resources/vector/Resolution.svg").c_str(),0.03);
    ResolutionLabel.setIconPositionLeft();

    Clabel ZoomLabel(renderer,{400,400},true,true,font);
    ZoomLabel.LoadSVGtoLabel((execDir+"/resources/vector/Zoom.svg").c_str(),0.03);
    ZoomLabel.setIconPositionLeft();

    Clabel TimeLabel(renderer,{400,400},true,true,font);
    TimeLabel.LoadSVGtoLabel((execDir+"/resources/vector/Date.svg").c_str(),0.03);
    TimeLabel.setIconPositionLeft();


    Clabel FileLabel(renderer,{400,400},true,true,font);
    FileLabel.LoadSVGtoLabel((execDir+"/resources/vector/File.svg").c_str(),0.03);
    FileLabel.setIconPositionLeft();


    Clabel UnhideTipLabel(renderer,{400,400},true,true,font);


    Clabel InfoLabel(renderer,{400,400},true,false,font);
    CDebugLabels DebugLabel(renderer,{400,400},font);

    Clabel debugline(renderer,Cordinates{0,0},true,false,true,font,SDL_Color{255,255,255,255});
    debugline.setBackgroundColor({255,0,0,255});
    Clabel debugline2(renderer,Cordinates{0,0},true,false,true,font,SDL_Color{255,255,255,255});
    debugline2.setBackgroundColor({255,0,0,255});

    Clabel debuglineimg(renderer,Cordinates{0,0},true,false,true,font,SDL_Color{0,255,255,255});
    debuglineimg.setBackgroundColor({0,255,255,255});
    Clabel debugline2img(renderer,Cordinates{0,0},true,false,true,font,SDL_Color{0,255,255,255});
    debugline2img.setBackgroundColor({0,255,255,255});

    std::shared_ptr<CButton> NextImageRightButton =  std::make_shared<CButton>("",renderer,Cordinates{200,200},true,true,true,font,SDL_Color{64,255,64,255});
   // CButton NextImageRightButton("",renderer,{200,200},true,true,true,font,{64,255,64,255});
    NextImageRightButton->setSvgIcon((execDir+"/resources/vector/ArrowRight.svg").c_str(),false,0.06);



    std::shared_ptr<CButton> NextImageLeftButton =  std::make_shared<CButton>("",renderer,Cordinates{200,200},true,true,true,font,SDL_Color{64,255,64,255});
    //CButton NextImageLeftButton("",renderer,{200,200},true,true,true,font,{64,255,64,255});
    NextImageLeftButton->setSvgIcon((execDir+"/resources/vector/ArrowLeft.svg").c_str(),false,0.06);


     std::shared_ptr<CButton> FullscreenButton = std::make_shared<CButton>("",renderer,Cordinates{200,200},true,true,true,font,SDL_Color{64,255,64,255});
    //CButton NextImageLeftButton("",renderer,{200,200},true,true,true,font,{64,255,64,255});
     FullscreenButton->setSvgIcon((execDir+"/resources/vector/Fullscreen.svg").c_str(),false,0.06);

    //  Clabel(SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){
    CAnimatedlabel AnimLabelOnCopy(renderer,{0,0},true,true,true,font,{0,255,0,255},2,"Copied to clipboard");

    CButtonHbox ButtonsHbox;

    
    ButtonsHbox.addButton(NextImageLeftButton);
    ButtonsHbox.addButton(FullscreenButton);
    ButtonsHbox.addButton(NextImageRightButton);
    

    SDL_Event event;

    CBackground background(renderer);
    //SDL_Texture* backgroundTexture = CreateRadialGradientTexture(renderer, winW, winH,{0,0,0,255});


    Uint32 lastTime = SDL_GetTicks();
    SDL_DisplayMode mode;
     int thumb_showing=0;


     CFrameControl FrameControl(2,true);
     FrameControl.ResetCoolDown(2);
     FileScanner.startWatching();


     bool imageToCenter=false;

     
    //main loop
    CClipboard Clipboard;
    CCursor Cursor;

    CCursor::cursorType CursorType ;
    
    while (running) {

        

       CursorType=CCursor::Arrow;

        if(FileScanner.hasNewImages()){


            thumbgroup.addThumbnail(FileScanner.getLastImageFile());
            Images->addImage(FileScanner.getLastImageFile());
        };



        //DES_FPS=10;
        FrameControl.makeAllFalse();
        
        int displayIndex = SDL_GetWindowDisplayIndex(window);
        
        
        SDL_GetCurrentDisplayMode(displayIndex, &mode);

        int refreshRate = mode.refresh_rate;

        if(windowActive){

            DES_FPS=165;
        }else {
            DES_FPS=10;
        }

        //FrameControl.setWindowActive(windowActive)
        FrameControl.setScrolling(dragging);
       
    
        Uint32 frameStart = SDL_GetTicks();
        auto start = std::chrono::high_resolution_clock::now();

        SDL_GetRendererOutputSize(renderer, &winW, &winH);



       
        background.StartLerp({thumbgroup.getThumbnailByInd(currentIndex)->getTavgcolor().r,thumbgroup.getThumbnailByInd(currentIndex)->getTavgcolor().g,thumbgroup.getThumbnailByInd(currentIndex)->getTavgcolor().b,255}, 0.5f);
        background.Update(deltaTime);
        background.Render();


        //std::cout<<"c image cordy "<<Images->getCurrentImageCords().y<<std::endl;
      if(imageToCenter){
       // Images->CenterCurrentImage(1000,700);
        imageToCenter=false;

      }

        Images->Render(winW,winH);
        currentIndex= Images->getCurrentIndex();
        zoom=Images->getCurrentImageZoom();
        Cordinates c=Images->getCurrentImageCords();
        offsetX=c.x;
        offsetY=c.y;

 

        
        thumb_showing=thumbgroup.getThumbShowing();


      





        // ---- Render info text (with black background)
        std::string info = "File: " + std::string(FileScanner.getImageFile(currentIndex)) +
        "  Size: " + std::to_string(Images->getCurrentImageW()) + "x" + std::to_string(Images->getCurrentImageH()) +
        "  Zoom: " + std::to_string((int)(zoom*100)) + "%";


      
        FileLabel.setVisibility(!hide_ui);
        TimeLabel.setVisibility(!hide_ui);
        ResolutionLabel.setVisibility(!hide_ui);
        ZoomLabel.setVisibility(!hide_ui);
        RotateRightLabel.setVisibility(!hide_ui);
        RotateLeftLabel.setVisibility(!hide_ui);
        RotateRightButton.setEnabled(!hide_ui);
        RotateLeftButton.setEnabled(!hide_ui);
        UnhideTipLabel.setVisibility(hide_ui);
        thumbgroup.setVisibility(!hide_ui);


         
       
        UnhideTipLabel.Render({0,winH-UnhideTipLabel.getLabelH()}, "Ctrl+H to unhide UI");
        std::string DisplayFilePath = FileScanner.getImageFile(currentIndex).substr(FileScanner.getImageFile(currentIndex).find_last_of((delim),FileScanner.getImageFile(currentIndex).length()));
        
        FileLabel.Render({0,winH-FileLabel.getLabelH()}, "File: " + DisplayFilePath.substr(1,DisplayFilePath.length()));
        TimeLabel.Render({0,FileLabel.getNexty()-FileLabel.getLabelH()*2}, Images->getCurrentImageTime());
        ResolutionLabel.Render({0,TimeLabel.getNexty()-TimeLabel.getLabelH()*2}, "Size: "+std::to_string(Images->getCurrentImageW()) + "x" + std::to_string(Images->getCurrentImageH()));
        ZoomLabel.Render({0,ResolutionLabel.getNexty()-ResolutionLabel.getLabelH()*2}, "Zoom: " + std::to_string((int)(zoom*100)) + "%");
        AnimLabelOnCopy.Render(0,THUMB_HEIGHT+4*THUMB_PADDING, deltaTime);


        int lthu=0;

        auto& tmg= thumbgroup.getThumbnails();
        for(auto& t :tmg){

            if(t->isLoaded()){lthu++;}
        }
       
        std::vector<std::string> strs;
        strs.push_back("MAX: "+std::to_string(refreshRate)+"|DES: "+std::to_string(FrameControl.estimateFrameDelat(refreshRate))+"|DP_index: "+std::to_string(displayIndex)+"|Fps: "+std::to_string(fps));
        strs.push_back("Preloaded thumbnails:"+std::to_string(lthu)+" "+std::to_string((lthu * 100) / FileScanner.getImageFilesSize()) + "%");
        strs.push_back("Image Rotation "+std::to_string(Images->getCurrentImageRotation()));
        strs.push_back("Image Loaded "+std::to_string(Images->getLoadedImages()));
        strs.push_back("Current image "+std::to_string(Images->getCurrentIndex()));
        strs.push_back("Load Queue  "+std::to_string(Images->getQueueSize()));
        strs.push_back("Ready surfaces  "+std::to_string(Images->getReadySurface()));
        strs.push_back("Mouse (x,y) "+std::to_string(lastMouseX)+","+std::to_string(lastMouseY));
        strs.push_back(info);
        strs.push_back("DeltaT  "+std::to_string(deltaTime));
        DebugLabel.setVisibility(debug_mode);
        DebugLabel.setCords(0, THUMB_WIDTH);
        DebugLabel.Render(strs);
        debugline.setVisibility(debug_mode);
        debugline2.setVisibility(debug_mode);
        debuglineimg.setVisibility(debug_mode);
        debugline2img.setVisibility(debug_mode);
        debugline.Render(Cordinates{winW/2-4,winH},Cordinates{9,winH});
        debugline2.Render(Cordinates{0,winH/2-4},Cordinates{winW,9});
        {

            int x=Images->getCurrentImageCords().x;
            int y=Images->getCurrentImageCords().y;

        debuglineimg.Render(Cordinates{winW/2-4+x,winH+y},Cordinates{9,winH});
        debugline2img.Render(Cordinates{0+x,winH/2-4+y},Cordinates{winW,9});
        }


           
        NextImageRightButton->setEnabled(!hide_ui);
        NextImageLeftButton->setEnabled(!hide_ui);
        FullscreenButton->setEnabled(!hide_ui);
        {

            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            thumbgroup.Render(winH, winW,mouseX,mouseY,deltaTime,CursorType);
            FrameControl.setMouseOnButton(NextImageRightButton->CheckIfHover(mouseX,mouseY,deltaTime));
            FrameControl.setMouseOnButton(NextImageLeftButton->CheckIfHover(mouseX,mouseY,deltaTime));
            FrameControl.setMouseOnButton(RotateLeftButton.CheckIfHover(mouseX,mouseY,deltaTime));
            FrameControl.setMouseOnButton(RotateRightButton.CheckIfHover(mouseX,mouseY,deltaTime));
            FrameControl.setMouseOnButton(FullscreenButton->CheckIfHover(mouseX,mouseY,deltaTime));
            //if(mouseY!=lastMouseY)
            FrameControl.setMouseOnThmbnails(mouseY<THUMB_WIDTH&&windowActive);

            if(FrameControl.getMouseOnButton()) CursorType=CCursor::Hand;
             //if(FrameControl.getMouseOnButton()) CursorType=CCursor::Hand;
            if(mouseY>=0 && mouseY<=10){CursorType=CCursor::SizeWE;}
            

        }

        RotateRightButton.Render(0,ZoomLabel.getNexty()-ZoomLabel.getLabelH()*2);

        RotateLeftButton.Render(0,RotateRightButton.getY()-RotateRightButton.getH());
       // NextImageRightButton->Render(winW/2,winH-NextImageRightButton->getH());
        //NextImageLeftButton->Render(winW/2-NextImageLeftButton->getW(),winH-NextImageLeftButton->getH());
        ButtonsHbox.Render( winW/2, winH);

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


               Images->CenterCurrentImage(winW,winH);
     

                //thumbgroup.setThumbShowing(thumb_showing);
                thumbgroup.ReplaceThumbnailsAround();
                thumbgroup.UlnoanLoad();

               // FrameControl.ResetCoolDown();
                FrameControl.ResetCoolDown(0.2f);

            }
             if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                windowActive = true;
                }

                if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                    windowActive = false;
                }
            if (event.window.event == SDL_WINDOWEVENT_MAXIMIZED || event.window.event == SDL_WINDOWEVENT_RESTORED)
            {

                std::cout << "Window was maximized!" << std::endl;

            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    running = false;

                if (event.key.keysym.sym == SDLK_f ) {
                    fullscreen = !fullscreen;
                    SDL_SetWindowFullscreen(
                        window,
                        fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
                    );
                    thumbgroup.UlnoanLoad();

                    FrameControl.ResetCoolDown();
                    //FrameControl.ResetCoolDown(0.2f);
                }
                if (event.key.keysym.sym == SDLK_r && (event.key.keysym.mod & KMOD_LSHIFT)) {
                    Images->CurrentImageRotate270(winW,winH);
                }else if(event.key.keysym.sym == SDLK_r){

                       Images->CurrentImageRotate90(winW,winH);
                }
                if (event.key.keysym.sym == SDLK_c && (event.key.keysym.mod & KMOD_CTRL)) {
                    //std::cout<<"Copy to clipboard init"<<std::endl;
               
                   // std::cout<<"good"<<std::endl;
                    
                    if(Images->getCurrentImageSurface()) {Clipboard.copyImageToClipboard(Images->getCurrentImageSurface());
                    AnimLabelOnCopy.ResetTimer();
                    FrameControl.ResetCoolDown(3);
                
                }
                }

                if (event.key.keysym.sym == SDLK_h &&(event.key.keysym.sym & KMOD_CTRL)) {
                    hide_ui=!hide_ui;
                }
                if (event.key.keysym.sym == SDLK_SPACE) {
                    //free_mode=!free_mode;
                    std::cout<<"free: "<<free_mode<<std::endl;


                }
                if (event.key.keysym.sym == SDLK_RIGHT) {
                    Loadthumbnails=true;
                    int ind=Images->NextImage(1,winW,winH);
                    thumbgroup.NextThumbnail(1,winW,winH);
                    FrameControl.ResetCoolDown();

                    


                }
                if (event.key.keysym.sym == SDLK_LEFT) {
                    Loadthumbnails=true;
                    int ind=Images->NextImage(-1,winW,winH);

                    thumbgroup.NextThumbnail(-1,winW,winH);
                    FrameControl.ResetCoolDown();
                    
                }
                if (event.key.keysym.sym == SDLK_UP) {
                  
        

                    

                    thumbgroup.UpdateScrollOffset(1,winW,winH);
                    thumbgroup.ReplaceThumbnailsAround();
                     FrameControl.ResetCoolDown(0.2f);
      
                }
                if (event.key.keysym.sym == SDLK_DOWN) {

   

                

                    thumbgroup.UpdateScrollOffset(-1,winW,winH);
                    thumbgroup.ReplaceThumbnailsAround();
                    FrameControl.ResetCoolDown(0.2f);



                }

                if (event.key.keysym.sym == SDLK_d) {

                    if(DEBUG) debug_mode=!debug_mode;


                }
               // std::cout << "tmb " <<currentIndex-thumbgroup.getScrollOffset() <<" scroll:"<<thumbgroup.getScrollOffset()<<" ind:"<<currentIndex<<std::endl;
                //std::cout<<"winW over "<<thumb_showing<<std::endl;



            }

            if(Loadthumbnails){

                thumbgroup.ReplaceThumbnailsAround();

                Loadthumbnails=false;
            }

        
            if (event.type == SDL_MOUSEWHEEL) {

                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                 FrameControl.setMouseOnScroll(true);
                if(mouseY>THUMB_HEIGHT+2*THUMB_PADDING || hide_ui){


                    float oldZoom = Images->getCurrentImageZoom();
                    zoom=oldZoom;
                    if (event.wheel.y > 0)
                        zoom *= 1.1f;
                    else if (event.wheel.y < 0)
                        zoom /= 1.1f;

                

                    float scaleChange = zoom / oldZoom;

                
                
                    offsetX = mouseX - scaleChange * (mouseX - offsetX);
                    offsetY = mouseY - scaleChange * (mouseY - offsetY);

                    Images->setCurrentImageZoom(zoom);
                    Images->setCurrentImageCords({(int)offsetX,(int)offsetY});

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
            NextImageRightButton->setMouseLocation(mouseX, mouseY);
            NextImageLeftButton->setMouseLocation(mouseX, mouseY);
            FullscreenButton->setMouseLocation(mouseX, mouseY);
            RotateLeftButton.setMouseLocation(mouseX,mouseY);
            RotateRightButton.setMouseLocation(mouseX,mouseY);

             



            int nimg= thumbgroup.CheckIfThumbnaiClicked(0, -1, mouseX, mouseY);

            if( RotateLeftButton.CheckIfClicked()){
                dragging=false;
                Images->CurrentImageRotate90(winW,winH);
            }

            if( RotateRightButton.CheckIfClicked()){
                dragging=false;
                Images->CurrentImageRotate270(winW,winH);
            }

            if(nimg!=-1){
                dragging=false;
                Loadthumbnails=true;
                int ind=Images->NextImage(nimg-currentIndex,winW,winH);
               

            }

            if(FullscreenButton->CheckIfClicked()){
                dragging = false;
                fullscreen = !fullscreen;
                    SDL_SetWindowFullscreen(
                        window,
                        fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
                    );
                    thumbgroup.UlnoanLoad();

                    FrameControl.ResetCoolDown();
                SDL_GetRendererOutputSize(renderer, &winW, &winH);
                //Centerhear
                imageToCenter=true;
                

            }

            if(NextImageRightButton->CheckIfClicked()|| NextImageLeftButton->CheckIfClicked()){
                dragging=false;
                Loadthumbnails=true;
                int ind=Images->NextImage(NextImageRightButton->CheckIfClicked()?1:-1,winW,winH);
                thumbgroup.NextThumbnail(NextImageRightButton->CheckIfClicked()?1:-1,winW,winH);
              
            }


                }else if (event.button.button == SDL_BUTTON_LEFT) {

                  if(event.button.y<=10){


                       thumbgroup.MoveScrollBar(event.button.x, event.button.y, winW, winH);
                        dragging=false;
                 }     

                }


                if (event.type == SDL_MOUSEBUTTONUP &&
                    event.button.button == SDL_BUTTON_LEFT) {
                    dragging = false;
                    }


                    if (event.type == SDL_MOUSEMOTION && dragging) {
                        int dx = event.motion.x - lastMouseX;
                        int dy = event.motion.y - lastMouseY;

                        
                        Images->moveCurrentImage(dx, dy);
                        lastMouseX = event.motion.x;
                        lastMouseY = event.motion.y;
                    }
        }
        if(dragging) CursorType=CCursor::SizeAll;

       Cursor.setCursor( CursorType);
        Uint32 currentTime = SDL_GetTicks();
     
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        //int frameDelay=estimateFrameDelat(DES_FPS);
        int frameDelay=FrameControl.estimateFrameDelat(refreshRate);
            if (frameDelay > frameTime) {
                SDL_Delay(frameDelay - frameTime);
            }

        auto end = std::chrono::high_resolution_clock::now();

  
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        fps=round((float)1000/(duration.count()/1000));
        deltaTime = (currentTime - lastTime) / 1000.0f; 
        lastTime = currentTime;
        FrameControl.UpdateCoolDown(deltaTime);


   

    }



    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    TTF_CloseFont(font);
    TTF_Quit();
    FileScanner.stopWatching();

    return 0;
}



#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int) {
    // Get the command line arguments
    int argc = 1; // The first argument is always the program name
    wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (argc < 2) {
        // Convert the wide string to a narrow string for MessageBox
        const wchar_t* errorMsg = L"Usage: viewer <image_path>";
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, errorMsg, -1, NULL, 0, NULL, NULL);
        char* errorMsgA = new char[size_needed];
        WideCharToMultiByte(CP_UTF8, 0, errorMsg, -1, errorMsgA, size_needed, NULL, NULL);

        MessageBoxA(NULL, errorMsgA, "Error", MB_OK | MB_ICONERROR);

        delete[] errorMsgA;
        return 1;
    }

    // Convert the command line arguments from wchar_t** to char**
    char** argv = new char*[argc];
    for (int i = 0; i < argc; ++i) {
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, NULL, 0, NULL, NULL);
        argv[i] = new char[size_needed];
        WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, argv[i], size_needed, NULL, NULL);
    }

    
    // Call your original main function
    int result = main(argc, argv);

    // Free the argument list
    for (int i = 0; i < argc; ++i) {
        delete[] argv[i];
    }
    delete[] argv;

    // Free the wide-char arguments
    LocalFree(wargv);
    //system("pause");
    return result;

}
#endif