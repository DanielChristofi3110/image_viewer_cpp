#include "globals.hpp"
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>






class CFrameControl{

    private:
        bool WindowActive=false;
        bool MouseOnButton=false;
        bool MouseOnScroll=false;
        bool MouseOnThmbnails=false;
        bool Scrolling=false;
        float time=2;
        float cctime=2;
        int idleFps=0;
        bool enabled=false;
    
    
    public:

    CFrameControl(int t,bool e,int ifps){
        enabled =e;
        time=t;
        cctime=-1;
        idleFps=ifps;
    }

    void setWindowActive(bool b){
        if(b)ResetCoolDown();
        WindowActive=b;}

    void setMouseOnButton(bool b){
        if (MouseOnButton) return;
        if(b)ResetCoolDown();
        MouseOnButton=b;}

    void setMouseOnScroll(bool b)
    {
        if(b)ResetCoolDown();
        MouseOnScroll=b;}

    void setMouseOnThmbnails(bool b)
    {
       if(b) ResetCoolDown();
        MouseOnThmbnails=b;}

    void setScrolling(bool b){
      if(b)  ResetCoolDown();
        Scrolling=b;}


    void makeAllFalse(){

        WindowActive=false;
         MouseOnButton=false;
         MouseOnScroll=false;
         Scrolling=false;
         MouseOnThmbnails=false;

    }


    bool getMouseOnButton(){


        return MouseOnButton;
    }
    int estimateFrameDelat(int mfps){
        int dfps=mfps;
         //std::cout<<"sss\n";
         if(!enabled) return 1000/dfps;

        if(cctime>0){
            dfps=mfps;

        }else if(MouseOnButton){

            dfps=mfps;
        }else if(MouseOnScroll){

            dfps=mfps;
        }else if(Scrolling){

            dfps=mfps;
           
        }else{

            dfps=idleFps;
        }



        return 1000/dfps;

    }

    void UpdateCoolDown(float dt){
        if(cctime<0) return;
        cctime-=dt;

        


    }

    void  ResetCoolDown(){

        cctime=time;
    }

      void  ResetCoolDown(float f){

        cctime=f;
    }




    





};