#pragma once
#include "globals.hpp"
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>




class CFileScanner{
private:
    std::vector<fs::path> imageFiles;
    std::unordered_set<fs::path> knownImages;
    fs::path firstImagePath;
    fs::path dir;

    std::vector<std::string> exts = {".png", ".jpg", ".jpeg", ".bmp"};

    float updateTime = 1;
    float cupdateTime = 1;
    std::thread scannerThread;
    std::mutex dataMutex;
    std::atomic<bool> running{false};
    std::atomic<bool> foundNew{false};
    bool single_image_load;
public:

    CFileScanner(char* ar, float t,bool sil){
        updateTime = t;
        cupdateTime = t;
        single_image_load=sil;

        if (ar == nullptr) {
        std::cerr << "Error: input path is null\n";
        return;
    }


        fs::path fi(ar);
        firstImagePath = fi;
        dir = firstImagePath.parent_path();

       // loadImages();
     
        loadImages();
        sortImages();
      
    }



    CFileScanner( std::unique_ptr<std::filesystem::path> p, float t,bool sil){
        updateTime = t;
        cupdateTime = t;
        single_image_load=sil;


        fs::path fi=std::move(*p);
        firstImagePath = fi;
        dir = firstImagePath.parent_path();

       // loadImages();
        loadImages();
       
 
        sortImages();
       
    }
    ~CFileScanner(){


        
    }

    fs::path getFirstimagePath(){return firstImagePath;}

    void loadImages(){

        for (const auto& entry : fs::directory_iterator(dir)) {

            if (!entry.is_regular_file()) continue;

            auto p = entry.path();

            std::string ext = p.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            if (std::find(exts.begin(), exts.end(), ext) != exts.end()) {

                if (DEBUG)
                    std::cout << "Loaded image " << p.string() << std::endl;

                imageFiles.push_back(p);
                knownImages.insert(p);
            }
        }
    }

    void loadFirstImage() {

        if (!fs::exists(firstImagePath) || !fs::is_regular_file(firstImagePath))
            return;

        std::string ext = firstImagePath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (std::find(exts.begin(), exts.end(), ext) != exts.end()) {

            if (DEBUG)
                std::cout << "Loaded initial image " << firstImagePath.string() << std::endl;

            imageFiles.push_back(firstImagePath);
            knownImages.insert(firstImagePath);
        }
    }

    void sortImages(){

        std::sort(imageFiles.begin(), imageFiles.end());
    }


    bool checkForNewImage(float dt){

        bool foundNew = false;

        if (cupdateTime > 0){
            cupdateTime -= dt;
            return false;
        }

        cupdateTime = updateTime;

        std::cout << "checking file Update " << std::endl;

        for (const auto& entry : fs::directory_iterator(dir))
        {
            if (!entry.is_regular_file()) continue;

            auto p = entry.path();

            std::string ext = p.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            if (std::find(exts.begin(), exts.end(), ext) == exts.end())
                continue;

            if (knownImages.find(p) == knownImages.end())
            {
                if (DEBUG)
                    std::cout << "New image detected: " << p.string() << std::endl;

                imageFiles.push_back(p);
                knownImages.insert(p);
                foundNew = true;
            }
        }

        return foundNew;
    }

    void scanLoop()
    {
        while (running)
        {

            //std::cout<<"scan loop"<<std::endl;
            for (const auto& entry : fs::directory_iterator(dir))
            {
                if (!entry.is_regular_file()) continue;

                auto p = entry.path();

                std::string ext = p.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (std::find(exts.begin(), exts.end(), ext) == exts.end())
                    continue;

                std::lock_guard<std::mutex> lock(dataMutex);

                if (knownImages.find(p) == knownImages.end())
                {
                    if (DEBUG)
                        std::cout << "New image detected: " << p.string() << std::endl;

                    imageFiles.push_back(p);
                    knownImages.insert(p);
                    foundNew = true;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds((int)(updateTime*1000)));
        }
    }

    void startWatching(){
        if(single_image_load) return;
        running = true;
        scannerThread = std::thread(&CFileScanner::scanLoop, this);
         std::cout<<"File scanner loop started"<<std::endl;
    }
    
    void stopWatching(){
        running = false;

        if (scannerThread.joinable())
            scannerThread.join();
           std::cout<<"File scanner loop ended"<<std::endl;
    }

    int getInitCurrentIndex(){
        if(single_image_load) return 0;
        for (size_t i = 0; i < imageFiles.size(); i++) {

            if (imageFiles[i] == firstImagePath)
                return i;
        }

        return 0;
    }


    const std::vector<std::string> getImageFiles() const {
        
        std::vector<std::string> result;
        if(single_image_load){

            result.push_back(firstImagePath.string());
            return result;
        }
        result.reserve(imageFiles.size());

        for (const auto& p : imageFiles)
            result.push_back(p.string());

        return result;
    }

    bool hasNewImages()
    {
        return foundNew.exchange(false);
    }

    int getImageFilesSize(){
        if(single_image_load) return 1;
          std::lock_guard<std::mutex> lock(dataMutex);
        return imageFiles.size();
    }


    const std::string getImageFile(int i) {
        std::lock_guard<std::mutex> lock(dataMutex);
        return imageFiles[i].string();
    }


    const std::string getLastImageFile(){
        
        std::lock_guard<std::mutex> lock(dataMutex);
        return imageFiles.back().string();
    }


int addPath(std::filesystem::path&& p){
    imageFiles.push_back(std::move(p));
    knownImages.insert(imageFiles.back());
    return imageFiles.size()-1;
}

};