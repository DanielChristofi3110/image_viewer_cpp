
#include "globals.hpp"
#include "thumbnails.hpp"
#include "image.hpp"
#include "GUI.hpp"
#include "background.hpp"
#include "FrameControl.hpp"
#include "FileScanner.hpp"
#include "Clipboard.hpp"
#include "Cursor.hpp"
#include "ConfigLoader.hpp"
#include "Canvas.hpp"
#include "WindowDecorations.hpp"
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_events.h>
#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_set>
#include <vector>

#ifdef _WIN32
#include <dwmapi.h>
#include<Windows.h>
#endif
#ifdef __linux__
#include <unistd.h>
#include <limits.h>
#endif


#define BORDERLESS false


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

std::string getConfigPath()
{
    const char* home = std::getenv("HOME");
    if (!home)
    {
        throw std::runtime_error("HOME environment variable not set");
    }

    std::string base = std::string(home) + "/.config/imageviewer/";
    return base + "config.ini";
}
#endif


int main(int argc, char* argv[]) {
   // std::cout << "Version: " << APP_VERSION << std::endl;
    #ifdef _WIN32
    SetProcessDPIAware();
    #endif

    execDir=getExecutableDirectory();
    #ifdef _WIN32
    resDir=execDir;
    confDir=execDir;
    #endif

    confDir=execDir+"/config/config.ini";
    #ifdef __linux
    if(BORDERLESS)SDL_SetHint(SDL_HINT_VIDEODRIVER, "x11");
    resDir=execDir+"/../share/imageviewer/";
    confDir=getConfigPath();
    #ifndef NDEBUG
    std::cout << "Debug mode\n";
    resDir=execDir;
    confDir=execDir+"/config/config.ini";
  
    #endif
    #endif

    //SDL_SetHint(SDL_HINT_VIDEODRIVER, "x11");//nowayland
    //SDL_SetHint(SDL_HINT_VIDEO_WAYLAND_PREFER_LIBDECOR, "1");
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    SDL_Surface* icon = IMG_Load((resDir+"/resources/images/iconimage.png").c_str());

    
    //AA
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    SDL_Window* window = SDL_CreateWindow(
        "Image Viewer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1000, 700,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE| SDL_WINDOW_ALLOW_HIGHDPI 
    );
    if(BORDERLESS)SDL_SetWindowBordered(window, SDL_FALSE);
    std::cout<<"------------------Created window-----------------------"<<std::endl;

    if (icon) {
     std::cout<<"Icon\n";
    SDL_SetWindowIcon(window, icon);
    SDL_FreeSurface(icon);
    }
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

    std::cout <<"HELLO"<<std::endl;
    std::shared_ptr<CConfigLoader> ConfigLoader;
    ConfigLoader=std::make_shared<CConfigLoader>();
    std::cout <<confDir<<std::endl;
    if (ConfigLoader->load(confDir))
    {
        std::cout << "Font Name: " << ConfigLoader->getFontName() << std::endl;
        std::cout << "Font Size: " << ConfigLoader->getFontSize() << std::endl;
        std::cout << "Idle Fps: " << ConfigLoader->getidleFps() << std::endl;
    }

    if (TTF_Init() == -1) {
        std::cout << "TTF Init Error: " << TTF_GetError() << "\n";
        return 1;
    }



    int idleFps=ConfigLoader->getidleFps();
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE
    );

    
        ASYNCLOADING= ConfigLoader->getASYNCLOADING();
        UNLOADAT= ConfigLoader->getUNLOADAT();
        MAXIMAGE_QUEUE=ConfigLoader->getMAXIMAGE_QUEUE();
        hide_ui=ConfigLoader->getHIDE_UI();

        std::cout<<"ASYNCLOADING  "<<ASYNCLOADING<<std::endl; 
        std::cout<<"UNLOADAT "<<UNLOADAT<<std::endl;
        std::cout<<"MQ "<<MAXIMAGE_QUEUE<<std::endl;



    
    std::cout<<"------------------Created  render-----------------------"<<std::endl;


   
    //font

    TTF_Font* font = TTF_OpenFont((resDir+"/fonts/"+ConfigLoader->getFontName()).c_str(), ConfigLoader->getFontSize());
    if (!font) {
        std::cout << "Failed to load font: " << TTF_GetError() <<"fallback"<< "\n";
        font = TTF_OpenFont((resDir+"/fonts/InterVariable.ttf").c_str(), ConfigLoader->getFontSize());
         if (!font)
             return 1;
    }

    CConfigEditorGUI ConfigEditorGUI(renderer,font,ConfigLoader);
     ConfigEditorGUI.loadFromConfig();
     ConfigEditorGUI.setEnabled(false);
    std::cout<<"------------------Loaded font-----------------------"<<std::endl;
    //int thumb_proc_ind = 0;
    //const int imageFiles_size = imageFiles.size();

 //Canvas

    CCanvas Canvas(renderer,font,(resDir+"/resources/vector/Pen.svg").c_str());
    std::cout<<"------------------Loaded Thumbnails-----------------------"<<std::endl;

    

    bool running = true;
    bool fullscreen = false;
    bool dragging = false;
   // bool window_dragging=true;

    int winW, winH;
    SDL_Event event;
    SDL_GetRendererOutputSize(renderer, &winW, &winH);

    std::unique_ptr<std::filesystem::path> droppedPath;

    CFrameControl FrameControl(2,true,idleFps);

    {
    Clabel dropImageLabel("Drop an Image file",renderer,{1000/2,700/2},true,true,true,font,{255,255,255,255});
    dropImageLabel.setBackgroundColor({0,0,0,255});

    std::unique_ptr<CImage> initBackImage=std::make_unique<CImage>(renderer);
    initBackImage->LoadImage_((resDir+"/resources/images/iconimage.png").c_str(),winW,winH);


    if (argc < 2) {
        std::cout << "Usage: viewer <image_path>\n";
        while (running) {
        // Handle events
        Uint32 frameStart = SDL_GetTicks();
        auto start = std::chrono::high_resolution_clock::now();
            while (SDL_PollEvent(&event)) {

                SDL_GetRendererOutputSize(renderer, &winW, &winH);
                if (event.type == SDL_QUIT) {
                    running = false;
                }

                if (event.type == SDL_DROPFILE) {
                droppedPath = std::make_unique<std::filesystem::path>(event.drop.file);

                //std::cout << "File dropped: " << droppedPath << std::endl;


                
                SDL_free(event.drop.file); 
                running = false;
            }
            
        }
        

        SDL_SetRenderDrawColor(renderer, 66, 66, 255, 255);
        

        SDL_RenderClear(renderer);
        //SDL_GetRendererOutputSize(renderer, &winW, &winH);
        initBackImage->Render(winW,winH);
        dropImageLabel.Render(Cordinates{winW/2 -100,winH/2});
        //dropImageLabel.Render(Cordinates{100,100});
        
        SDL_RenderPresent(renderer);
         Uint32 frameTime = SDL_GetTicks() - frameStart;
        //int frameDelay=estimateFrameDelat(DES_FPS);
        int frameDelay=FrameControl.estimateFrameDelat(1);
            if (frameDelay > frameTime) {
                SDL_Delay(frameDelay - frameTime);
            }
    }
    
    }else {
    droppedPath=std::make_unique<std::filesystem::path>(argv[1]);
    }

    if(droppedPath==nullptr){ 
        
        
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        IMG_Quit();
        SDL_Quit();
        TTF_CloseFont(font);
        TTF_Quit();
        return 1;}

    }
   running=true;
 
    //initBackImage.release();
    //std::cout<<"AAAA"<<std::endl;
  
    // image load f
    //const char * p= droppedPath.string().c_str();
    CFileScanner FileScanner(std::move(droppedPath),1,ConfigLoader->getSingle_image_load());

    // std::cout<<"------------------Images Loaded-----------------------"<<std::endl;
  
    // std::cout<<"------------------Images Sorted-----------------------"<<std::endl;

    int currentIndex = 0;

    //int thumbcurrentIndex=0;
  
    currentIndex=FileScanner.getInitCurrentIndex();
  
    //CImages Images(renderer,imageFiles,currentIndex,winW,winH);
    std::shared_ptr<CImages> Images = std::make_shared<CImages>(renderer,FileScanner.getImageFiles(),currentIndex,winW,winH);

    

    CWindowDecorations WindowDecorations(renderer,font,BORDERLESS,ConfigLoader->getMainColor());
    //Images->setCurrentImageWindowDecorationY(WindowDecorations.getH());
    WindowDecorations.SetCloseSVG(resDir+"/resources/vector/Close.svg");
    WindowDecorations.SetMaximizeSVG(resDir+"/resources/vector/Maximize.svg");
    WindowDecorations.SetMinimizeSVG(resDir+"/resources/vector/Minimize.svg");
    WindowDecorations.SetOptionSVG(resDir+"/resources/vector/Options.svg");
 
    float zoom = 0;

    float offsetX = 0;
    float offsetY = 0;

    int lastMouseX = 0;
    int lastMouseY = 0;

    float Thumbnail_ANIM_SPEED=ConfigLoader->getTHUMBNAIL_ANIMATION_SPEED();
   
    Images->LoadAroundAsync(ASYNCLOADING);
    CThumbnailGroup thumbgroup(FileScanner.getImageFilesSize(),renderer,Images,font,true,FileScanner.getImageFiles());
    thumbgroup.ReplaceThumbnailsAround(currentIndex,winW/THUMB_WIDTH);
    thumbgroup.setCurrentIndex(currentIndex);
    thumbgroup.MoveScrollTo(currentIndex, winW, winH);
    
   


    // Clabel RotateLeftLabel(renderer,{400,400},true,true,font);
    // RotateLeftLabel.LoadSVGtoLabel((resDir+"/resources/vector/RotateLeft.svg").c_str(),0.03f);
    // RotateLeftLabel.setIconPositionLeft();

    CButton RotateLeftButton("R",renderer,{400,400},true,true,true,font,{255,255,255,255});
    RotateLeftButton.setSvgIcon((resDir+"/resources/vector/RotateLeft.svg").c_str(), true,ConfigLoader->getMainColor(),0.03f);



    CButton RotateRightButton("Shift+R",renderer,{400,400},true,true,true,font,{255,255,255,255});
    RotateRightButton.setSvgIcon((resDir+"/resources/vector/RotateRight.svg").c_str(), true,ConfigLoader->getMainColor(),0.03f);

    CButton PenButton("Drawing Pen",renderer,{400,400},true,true,true,font,{255,255,255,255});
    PenButton.setSvgIcon((resDir+"/resources/vector/Pen.svg").c_str(), true,ConfigLoader->getMainColor(),0.03f);
    
    CButton PenColorButton("Change Color Right Click",renderer,{400,400},true,true,true,font,{255,255,255,255});
    //PenButton.setSvgIcon((resDir+"/resources/vector/Pen.svg").c_str(), true,0.03f);



    // Clabel RotateRightLabel(renderer,{400,400},true,true,font);
    // RotateRightLabel.LoadSVGtoLabel((resDir+"/resources/vector/RotateRight.svg").c_str(),0.03f);
    // RotateRightLabel.setIconPositionLeft();



    Clabel ResolutionLabel(renderer,{400,400},true,true,font);
    ResolutionLabel.LoadSVGtoLabel((resDir+"/resources/vector/Resolution.svg").c_str(),ConfigLoader->getMainColor(),0.03f);
    ResolutionLabel.setIconPositionLeft();

    Clabel ZoomLabel(renderer,{400,400},true,true,font);
    ZoomLabel.LoadSVGtoLabel((resDir+"/resources/vector/Zoom.svg").c_str(),ConfigLoader->getMainColor(),0.03f);
    ZoomLabel.setIconPositionLeft();

    Clabel TimeLabel(renderer,{400,400},true,true,font);
    TimeLabel.LoadSVGtoLabel((resDir+"/resources/vector/Date.svg").c_str(),ConfigLoader->getMainColor(),0.03f);
    TimeLabel.setIconPositionLeft();


    Clabel FileLabel(renderer,{400,400},true,true,font);
    FileLabel.LoadSVGtoLabel((resDir+"/resources/vector/File.svg").c_str(),ConfigLoader->getMainColor(),0.03f);
    FileLabel.setIconPositionLeft();


    Clabel UnhideTipLabel("Crl+H Unhide UI",renderer,{400,400},true,true,true,font,{255,255,255,255});


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
    NextImageRightButton->setSvgIcon((resDir+"/resources/vector/ArrowRight.svg").c_str(),false,ConfigLoader->getMainColor(),0.06f);



    std::shared_ptr<CButton> NextImageLeftButton =  std::make_shared<CButton>("",renderer,Cordinates{200,200},true,true,true,font,SDL_Color{64,255,64,255});
    //CButton NextImageLeftButton("",renderer,{200,200},true,true,true,font,{64,255,64,255});
    NextImageLeftButton->setSvgIcon((resDir+"/resources/vector/ArrowLeft.svg").c_str(),false,ConfigLoader->getMainColor(),0.06f);


     std::shared_ptr<CButton> FullscreenButton = std::make_shared<CButton>("",renderer,Cordinates{200,200},true,true,true,font,SDL_Color{64,255,64,255});
    //CButton NextImageLeftButton("",renderer,{200,200},true,true,true,font,{64,255,64,255});
     FullscreenButton->setSvgIcon((resDir+"/resources/vector/Fullscreen.svg").c_str(),false,ConfigLoader->getMainColor(),0.06f);



    std::shared_ptr<CButton> OptionsButton = std::make_shared<CButton>("",renderer,Cordinates{200,200},true,true,true,font,SDL_Color{64,255,64,255});
    //CButton NextImageLeftButton("",renderer,{200,200},true,true,true,font,{64,255,64,255});
     OptionsButton->setSvgIcon((resDir+"/resources/vector/Options.svg").c_str(),false,ConfigLoader->getMainColor(),0.06f);

    //  Clabel(SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){
    CAnimatedlabel AnimLabelOnCopy(renderer,{0,0},true,true,true,font,{0,255,0,255},2,"Copied to clipboard");

    CButtonHbox ButtonsHbox;

    
    ButtonsHbox.addButton(NextImageLeftButton);
    ButtonsHbox.addButton(FullscreenButton);
    ButtonsHbox.addButton(NextImageRightButton);
    

 

    CBackground background(renderer);
    //SDL_Texture* backgroundTexture = CreateRadialGradientTexture(renderer, winW, winH,{0,0,0,255});


    Uint32 lastTime = SDL_GetTicks();
    SDL_DisplayMode mode;
     int thumb_showing=0;


     
     FrameControl.ResetCoolDown(2);
     FileScanner.startWatching();


     bool imageToCenter=false;

     
    //main loop
    CClipboard Clipboard;
    CCursor Cursor;

    CCursor::cursorType CursorType ;

    CMouseLabel MouseLable(renderer,{0,0},true,true,true,font,{0,255,0,255});
 //    CTextBox(SDL_Renderer* r,Cordinates c,bool db, bool abs,bool v,TTF_Font * f,SDL_Color tc){
    CTextBox TextTextBox(renderer,{400,400},true,true,true,font,{0,0,0,0});
    
   


    //current

   
    

    bool Typing=false;
    bool DrawingMode=false;
    int drawingThickness=1;
    //Canvas.setPenInvertedColor({255,255,255,255});
    
    while (running) {

        
        Uint32 frameStart = SDL_GetTicks();
        auto start = std::chrono::high_resolution_clock::now();
       CursorType=CCursor::Arrow;

        if(FileScanner.hasNewImages()){


            thumbgroup.addThumbnail(FileScanner.getLastImageFile());
            Images->addImage(FileScanner.getLastImageFile());
        };



        //DES_FPS=10;
        FrameControl.makeAllFalse();
        FrameControl.setDrawingMode(DrawingMode);
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
        //FrameControl.setWindowActive(windowActive);
       
    
        

        SDL_GetRendererOutputSize(renderer, &winW, &winH);
        Images->setCurrentImageWindowDecorationY(WindowDecorations.getH());

        Images->set_window(winW,winH);

       
        background.StartLerp({thumbgroup.getThumbnailByInd(currentIndex)->getTavgcolor().r,thumbgroup.getThumbnailByInd(currentIndex)->getTavgcolor().g,thumbgroup.getThumbnailByInd(currentIndex)->getTavgcolor().b,255}, 0.5f);
        background.Update(deltaTime);
        background.Render();
        Canvas.setPenInvertedColor({thumbgroup.getThumbnailByInd(currentIndex)->getTavgcolor().r,thumbgroup.getThumbnailByInd(currentIndex)->getTavgcolor().g,thumbgroup.getThumbnailByInd(currentIndex)->getTavgcolor().b,255});
        WindowDecorations.setBackgroundColor({thumbgroup.getThumbnailByInd(currentIndex)->getTavgcolor().r,thumbgroup.getThumbnailByInd(currentIndex)->getTavgcolor().g,thumbgroup.getThumbnailByInd(currentIndex)->getTavgcolor().b,255});
        


        //std::cout<<"c image cordy "<<Images->getCurrentImageCords().y<<std::endl;
      if(imageToCenter){
       // Images->CenterCurrentImage(1000,700);
        imageToCenter=false;

      }
       
        Images->Render(winW,winH);
       
        currentIndex= Images->getCurrentIndex();
        zoom=Images->getCurrentImageZoom();
        
        Cordinates c=Images->getCurrentImageCords();
       // Images->setCurrentImageCords({c.x,c.y+WindowDecorations.getH()});
        //c=Images->getCurrentImageCords();
        offsetX=static_cast<float>(c.x);
        offsetY=static_cast<float>(c.y);
     
        Canvas.Render(static_cast<int>(offsetX),static_cast<int>(offsetY),zoom,0);//Images->getCurrentImageRotation());
 

        
        thumb_showing=thumbgroup.getThumbShowing();

        

        

      





        // ---- Render info text (with black background)
        std::string info = "File: " + std::string(FileScanner.getImageFile(currentIndex)) +
        "  Size: " + std::to_string(Images->getCurrentImageW()) + "x" + std::to_string(Images->getCurrentImageH()) +
        "  Zoom: " + std::to_string(static_cast<int>(std::floor(zoom*100))) + "%";


        thumbgroup.setVisibility(true);
        if(ConfigEditorGUI.isEnabled()){

            hide_ui=true;
            thumbgroup.setVisibility(false);
        }
        //hide_ui=ConfigLoader->getHIDE_UI();
        FileLabel.setVisibility(!hide_ui);
        TimeLabel.setVisibility(!hide_ui);
        ResolutionLabel.setVisibility(!hide_ui);
        ZoomLabel.setVisibility(!hide_ui);
       // RotateRightLabel.setVisibility(!hide_ui);
        //RotateLeftLabel.setVisibility(!hide_ui);
        PenButton.setEnabled(!hide_ui);
        PenColorButton.setEnabled(DrawingMode&&!hide_ui);
        RotateRightButton.setEnabled(!hide_ui);
        RotateLeftButton.setEnabled(!hide_ui);
        UnhideTipLabel.setVisibility(hide_ui);
        //thumbgroup.setVisibility(!hide_ui);


         
       
        UnhideTipLabel.Render(Cordinates{0,winH-UnhideTipLabel.getLabelH()});
        std::string DisplayFilePath = FileScanner.getImageFile(currentIndex).substr(FileScanner.getImageFile(currentIndex).find_last_of((delim),FileScanner.getImageFile(currentIndex).length()));
        
        FileLabel.Render({0,winH-FileLabel.getLabelH()}, "File: " + DisplayFilePath.substr(1,DisplayFilePath.length()));
        TimeLabel.Render({0,FileLabel.getNexty()-FileLabel.getLabelH()*2}, Images->getCurrentImageTime());
        ResolutionLabel.Render({0,TimeLabel.getNexty()-TimeLabel.getLabelH()*2}, "Size: "+std::to_string(Images->getCurrentImageW()) + "x" + std::to_string(Images->getCurrentImageH()));
        ZoomLabel.Render({0,ResolutionLabel.getNexty()-ResolutionLabel.getLabelH()*2}, "Zoom: " + std::to_string(static_cast<int>(std::floor(zoom*100))) + "%");
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
        OptionsButton->setEnabled(!hide_ui);

        thumbgroup.setWindowDecorationY(WindowDecorations.getH());
        {

            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            thumbgroup.Render(winH, winW,mouseX,mouseY,deltaTime,CursorType);
            FrameControl.setMouseOnButton(NextImageRightButton->CheckIfHover(mouseX,mouseY,deltaTime));
            FrameControl.setMouseOnButton(NextImageLeftButton->CheckIfHover(mouseX,mouseY,deltaTime));
            FrameControl.setMouseOnButton(RotateLeftButton.CheckIfHover(mouseX,mouseY,deltaTime));
            FrameControl.setMouseOnButton(PenButton.CheckIfHover(mouseX,mouseY,deltaTime));
             FrameControl.setMouseOnButton(PenColorButton.CheckIfHover(mouseX,mouseY,deltaTime));
            FrameControl.setMouseOnButton(RotateRightButton.CheckIfHover(mouseX,mouseY,deltaTime));
            FrameControl.setMouseOnButton(FullscreenButton->CheckIfHover(mouseX,mouseY,deltaTime));
            FrameControl.setMouseOnButton(OptionsButton->CheckIfHover(mouseX,mouseY,deltaTime));
            //if(mouseY!=lastMouseY)
            FrameControl.setMouseOnThmbnails(mouseY<THUMB_WIDTH&&windowActive);

            if((mouseY<THUMB_WIDTH+WindowDecorations.getH())&&windowActive){

                thumbgroup.UpdateYelevation(0,Thumbnail_ANIM_SPEED, deltaTime);
            }else if(hide_ui){
                thumbgroup.UpdateYelevation(THUMB_HEIGHT+THUMB_PADDING*2+5+WindowDecorations.getH(),Thumbnail_ANIM_SPEED, deltaTime);

            }

            if(FrameControl.getMouseOnButton()) CursorType=CCursor::Hand;
             //if(FrameControl.getMouseOnButton()) CursorType=CCursor::Hand;
            if(mouseY>=WindowDecorations.getH() && mouseY<=thumbgroup.getDrawProgressH()+WindowDecorations.getH()){
                MouseLable.Render(mouseX,mouseY,winW,winH,(std::to_string(1+static_cast<int>(std::floor((static_cast<float>(mouseX)/static_cast<float>(winW))*static_cast<float>(thumbgroup.getSize()))))+"/"+std::to_string(thumbgroup.getSize())));
                CursorType=CCursor::SizeWE;
                
            }

            TextTextBox.setVisible(debug_mode);
            TextTextBox.Render(winW,winH,mouseX,mouseY,deltaTime,CursorType);
            if(DrawingMode) Canvas.RenderPen(mouseX, mouseY,drawingThickness);
            
              WindowDecorations.Render(winW, winH, mouseX, mouseY, deltaTime,CursorType);

            // if(mouseY>WindowDecorations.getH()){

            //     window_dragging=false;
            // }
        }
        PenButton.Render(0,ZoomLabel.getNexty()-ZoomLabel.getLabelH()*2);
        RotateRightButton.Render(0,PenButton.getY()-PenButton.getH());
        RotateLeftButton.Render(0,RotateRightButton.getY()-RotateRightButton.getH());
        PenColorButton.Render(PenButton.getX()+PenButton.getW(),PenButton.getY());
        PenColorButton.setnColor({Canvas.getCurrentPenColor().r,Canvas.getCurrentPenColor().g,Canvas.getCurrentPenColor().b,128});
        PenColorButton.sethColor(Canvas.getCurrentPenColor());

       // NextImageRightButton->Render(winW/2,winH-NextImageRightButton->getH());
        //NextImageLeftButton->Render(winW/2-NextImageLeftButton->getW(),winH-NextImageLeftButton->getH());
        ButtonsHbox.Render( winW/2, winH);
        OptionsButton->Render(winW-OptionsButton->getW(),winH-OptionsButton->getH());
        {

            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            //ConfigEditorGUI.setEnabled(ConfEdit)
            ConfigEditorGUI.Render(winW, winH, mouseX, mouseY,deltaTime,CursorType);
        }
       
        
      
        SDL_RenderPresent(renderer);

        

        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT)
                running = false;

             if (event.type == SDL_TEXTINPUT) {
                if(Typing){
                //ttext += event.text.text;
               // std::cout<<"Tiped "<<ttext<<std::endl;
                //TextTextBox.appendtText(event.text.text);
                ConfigEditorGUI.AddTextToTyping(event.text.text);
                }
        }
            if (event.type == SDL_DROPFILE) {
                std::filesystem::path droppedPath(event.drop.file);

                std::cout << "File dropped: " << droppedPath.string() << std::endl;

                FileScanner.addPath(std::move(droppedPath));

                thumbgroup.addThumbnail(FileScanner.getLastImageFile());
                Images->addImage(FileScanner.getLastImageFile());

                SDL_free(event.drop.file);
            }

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

                //disable on typing
                if(!Typing){
                    SDL_StopTextInput();
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
                    Canvas.clear();
                }else if(event.key.keysym.sym == SDLK_r){

                       Images->CurrentImageRotate90(winW,winH);
                       Canvas.clear();
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
                if(event.key.keysym.sym == SDLK_z && (event.key.keysym.mod & KMOD_CTRL)){

                    Canvas.Undo();

                }

                if(event.key.keysym.sym == SDLK_d && (event.key.keysym.mod & KMOD_CTRL)){
                      if(DEBUG) debug_mode=!debug_mode;

                }else if (event.key.keysym.sym == SDLK_d) {

                  
                    DrawingMode=!DrawingMode;


                }
                


                //end Type
                }else{
                   
                    if (event.key.keysym.sym == SDLK_BACKSPACE) {
                        //TextTextBox.popText();
                        ConfigEditorGUI.POPTextToTyping();

                    }

                }
                // if (event.key.keysym.sym == SDLK_o) {
                //         //TextTextBox.popText();
                //         ConfigEditorGUI.setEnabled(! ConfigEditorGUI.isEnabled());

                // }
                
                if (event.key.keysym.sym == SDLK_RIGHT) {
                    Loadthumbnails=true;
                    //int ind=
                    std::cout<<"MQ "<<MAXIMAGE_QUEUE<<std::endl;
                    if(!(Images->getQueueSize()>MAXIMAGE_QUEUE)) {
                    Images->NextImage(1,winW,winH);
                    
                    Canvas.clear();
                    
                    thumbgroup.NextThumbnail(1,winW,winH);
                    FrameControl.ResetCoolDown();
                    }
                    


                }
                if (event.key.keysym.sym == SDLK_LEFT) {
                    Loadthumbnails=true;
                    //int ind=
                    if(!(Images->getQueueSize()>MAXIMAGE_QUEUE)) {
                    Images->NextImage(-1,winW,winH);
                    Canvas.clear();
                    

                    thumbgroup.NextThumbnail(-1,winW,winH);
                    FrameControl.ResetCoolDown();}
                    
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

               
               // std::cout << "tmb " <<currentIndex-thumbgroup.getScrollOffset() <<" scroll:"<<thumbgroup.getScrollOffset()<<" ind:"<<currentIndex<<std::endl;
                //std::cout<<"winW over "<<thumb_showing<<std::endl;



            }

            if(Loadthumbnails){

                thumbgroup.ReplaceThumbnailsAround();

                Loadthumbnails=false;
            }

            if (event.type == SDL_MOUSEWHEEL && DrawingMode) {
                // Check if Ctrl is held
                if (SDL_GetModState() & KMOD_CTRL) {
                    drawingThickness += event.wheel.y; // event.wheel.y is +1 or -1 depending on scroll
                    if (drawingThickness < 1) drawingThickness = 1; // prevent negative thickness
                }
            }
            if (event.type == SDL_MOUSEWHEEL ) {
                if(SDL_GetModState() & KMOD_CTRL) continue;
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                 FrameControl.setMouseOnScroll(true);

                

                if(mouseY>THUMB_HEIGHT+2*THUMB_PADDING){ //|| hide_ui){


                    float oldZoom = Images->getCurrentImageZoom();
                    zoom=oldZoom;
                    if (event.wheel.y > 0)
                        zoom *= 1.1f;
                    else if (event.wheel.y < 0)
                        zoom /= 1.1f;

                

                    float scaleChange = zoom / oldZoom;

                
                
                    offsetX = static_cast<float>(mouseX) - scaleChange * (static_cast<float>(mouseX) - offsetX);
                    offsetY = static_cast<float>(mouseY) - scaleChange * (static_cast<float>(mouseY) - offsetY);

                    Images->setCurrentImageZoom(zoom);
                    Images->setCurrentImageCords({static_cast<int>(std::floor(offsetX)),static_cast<int>(std::floor(offsetY))});

                }else {

                    std::cout<<"THUMB MOUSE SCROLL "<<event.wheel.y <<std::endl;

                    
                    thumbgroup.UpdateScrollOffset(event.wheel.y,winH,winW);
                }

        
            }
          
            //Right mouse button
            if((event.type == SDL_MOUSEBUTTONDOWN)&&  (event.button.button == SDL_BUTTON_RIGHT)){

                std::cout<<"Right click"<<std::endl;
                if(DrawingMode){
                    Canvas.nextColor();
                }




            }
            //ledt mouse button
            if (event.type == SDL_MOUSEBUTTONDOWN &&
                event.button.button == SDL_BUTTON_LEFT) {


           
            dragging = true&&!ConfigEditorGUI.isEnabled()&&!DrawingMode;
            
                
            lastMouseX = event.button.x;

            lastMouseY = event.button.y;




      
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

           // if(mouseY<WindowDecorations.getH()&& mouseY>0){ window_dragging=true;}

            if(DrawingMode)Canvas.StartStroke(static_cast<int>((mouseX - offsetX) / zoom), static_cast<int>((mouseY - offsetY) / zoom),drawingThickness);

            Typing=ConfigEditorGUI.isEnabled();
            //TextTextBox.CheckIfPressed(mouseX,mouseY,Typing);
            ConfigEditorGUI.checkIfAnyTyping(mouseX, mouseY);
            ConfigEditorGUI.checkIfButtonClick(mouseX, mouseY,confDir);


            if(Typing) SDL_StartTextInput();

            



            NextImageRightButton->setMouseLocation(mouseX, mouseY);
            NextImageLeftButton->setMouseLocation(mouseX, mouseY);
            FullscreenButton->setMouseLocation(mouseX, mouseY);
            OptionsButton->setMouseLocation(mouseX, mouseY);
            RotateLeftButton.setMouseLocation(mouseX,mouseY);
            RotateRightButton.setMouseLocation(mouseX,mouseY);
            PenButton.setMouseLocation(mouseX,mouseY);
            PenColorButton.setMouseLocation(mouseX,mouseY);
            
            if(WindowDecorations.CheckifCloseClick(mouseX,mouseY)){

                running=false;
            }
            //SDL_MinimizeWindow(window);

            if(WindowDecorations.CheckifMinimizeClick(mouseX,mouseY)){

                //running=false;
                SDL_MinimizeWindow(window);
            }
             
            if(WindowDecorations.CheckifMaximizeClick(mouseX,mouseY)){

                //running=false;
                //SDL_MinimizeWindow(window);
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



            int nimg=-1;
            
            nimg= thumbgroup.CheckIfThumbnaiClicked(0, -1, mouseX, mouseY);

            if( RotateLeftButton.CheckIfClicked()){
                dragging=false;
                Images->CurrentImageRotate90(winW,winH);
                Canvas.clear();
               
            }

            if( RotateRightButton.CheckIfClicked()){
                dragging=false;
                Images->CurrentImageRotate270(winW,winH);
                Canvas.clear();
               
            }

            if( PenButton.CheckIfClicked()){
                DrawingMode=!DrawingMode;
               
            }

            if( PenColorButton.CheckIfClicked()){
               if(DrawingMode) Canvas.nextColor();
               
            }

            if(nimg!=-1){
                dragging=false;
                Loadthumbnails=true;
                //int ind=
                Images->NextImage(nimg-currentIndex,winW,winH);
                Canvas.clear();
                //std::cout<<"nimg-currentIndex "<<nimg-currentIndex<<std::endl;
               

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

            if(OptionsButton->CheckIfClicked() || WindowDecorations.CheckifOptionClick(mouseX,mouseY)){
                
                ConfigEditorGUI.setEnabled(true);

            }

            if(NextImageRightButton->CheckIfClicked()|| NextImageLeftButton->CheckIfClicked()){
                dragging=false;
                Loadthumbnails=true;
                //int ind=
                Images->NextImage(NextImageRightButton->CheckIfClicked()?1:-1,winW,winH);
                Canvas.clear();
                thumbgroup.NextThumbnail(NextImageRightButton->CheckIfClicked()?1:-1,winW,winH);
                
            }


                }else if (event.button.button == SDL_BUTTON_LEFT) {

                  if(event.button.y<=WindowDecorations.getH()+thumbgroup.getDrawProgressH() && event.button.y>=0){


                       thumbgroup.MoveScrollBar(event.button.x, event.button.y, winW, winH);
                        dragging=false;
                 }     

                }


                if (event.type == SDL_MOUSEBUTTONUP &&
                    event.button.button == SDL_BUTTON_LEFT) {

                    Canvas.EndStroke();
                   // DrawingMode=false;   
                    dragging = false;
                   // window_dragging=false;
                    }


                    if (event.type == SDL_MOUSEMOTION && dragging) {
                        int dx = event.motion.x - lastMouseX;
                        int dy = event.motion.y - lastMouseY;

                        
                        Images->moveCurrentImage(dx, dy);

                        lastMouseX = event.motion.x;
                        lastMouseY = event.motion.y;
                    }
                    
                    // if (event.type == SDL_MOUSEMOTION && window_dragging) {
                    //     SDL_RaiseWindow(window);
                    //     int mx,my;
                    //     SDL_GetGlobalMouseState(&mx, &my);
                    //     int dx = mx;
                    //     int dy = my;

                    //     int wx, wy;
                    //     SDL_GetWindowPosition(window, &wx, &wy);

                    //     std::cout<<"Window Dragging "<<window_dragging<<" "<<wx<<" "<<dx<<std::endl;
                    //      SDL_SetWindowPosition(window, mx, mx);


                    //     lastMouseX = event.motion.x;
                    //     lastMouseY = event.motion.y;
                    // }

                if(event.type==SDL_MOUSEMOTION){

                    if(DrawingMode)Canvas.DrawOn(static_cast<int>(( event.motion.x - offsetX) / zoom), static_cast<int>(( event.motion.y - offsetY) / zoom));
                    //(mouseX-static_cast<int>(offsetX), mouseY-static_cast<int>(offsetY))
                }
        }
        if(dragging) CursorType=CCursor::SizeAll;
        if(DrawingMode) CursorType=CCursor::Hide;
        
       Cursor.setCursor( CursorType);
        Uint32 currentTime = SDL_GetTicks();
     
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        //int frameDelay=estimateFrameDelat(DES_FPS);
        Uint32 frameDelay=static_cast<Uint32>(FrameControl.estimateFrameDelat(refreshRate));
            if (frameDelay > frameTime) {
                SDL_Delay(frameDelay - frameTime);
            }

        auto end = std::chrono::high_resolution_clock::now();

  
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        fps=static_cast<int>(round((float)1000/(static_cast<float>(duration.count())/1000)));
        deltaTime = static_cast<float>(currentTime - lastTime) / 1000.0f; 
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

    // if (argc < 2) {
    //     // Convert the wide string to a narrow string for MessageBox
    //     const wchar_t* errorMsg = L"Usage: viewer <image_path>";
    //     int size_needed = WideCharToMultiByte(CP_UTF8, 0, errorMsg, -1, NULL, 0, NULL, NULL);
    //     char* errorMsgA = new char[size_needed];
    //     WideCharToMultiByte(CP_UTF8, 0, errorMsg, -1, errorMsgA, size_needed, NULL, NULL);

    //     MessageBoxA(NULL, errorMsgA, "Error", MB_OK | MB_ICONERROR);

    //     delete[] errorMsgA;
    //     return 1;
    // }

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
    #ifdef _DEBUG
    system("pause");
    #endif
    return result;

}
#endif