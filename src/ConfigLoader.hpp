#include <string>
#include <iostream>
#include "SimpleIni.h"

class CConfigLoader
{
private:
    std::string fontName;
    int fontSize = 0;
    int idleFps = 0;
    
    int aSYNCLOADING= 1;
    int uNLOADAT= 2;
    int mAXIMAGE_QUEUE=10;

public:
    bool load(const std::string& filename)
    {
        CSimpleIniA ini;
        ini.SetUnicode();

        SI_Error rc = ini.LoadFile(filename.c_str());
        if (rc < 0)
        {
            std::cerr << "Failed to load config file\n";
            return false;
        }

        const char* name = ini.GetValue("font", "name", "InterVariable.ttf");
        long size = ini.GetLongValue("font", "size", 18);
        long fps = ini.GetLongValue("settings", "idleFps", 10);
        long as = ini.GetLongValue("settings", "ASYNCLOADING", 1);
        long un = ini.GetLongValue("settings", "UNLOADAT", 2);
        long ma = ini.GetLongValue("settings", "MAXIMAGE_QUEUE", 10);

        fontName = name;
        fontSize = static_cast<int>(size);
        idleFps =static_cast<int>(fps);
        aSYNCLOADING= static_cast<int>(as);
        uNLOADAT= static_cast<int>(un);
        mAXIMAGE_QUEUE=static_cast<int>(ma);

        return true;
    }

    const std::string& getFontName() const
    {
        return fontName;
    }

    int getFontSize() const
    {
        return fontSize;
    }
     int getidleFps() const
    {
        return idleFps;
    }

     int getASYNCLOADING() const
    {
        return aSYNCLOADING;
    }

      int getUNLOADAT() const
    {
        return uNLOADAT;
    }
       int getMAXIMAGE_QUEUE() const
    {
        return mAXIMAGE_QUEUE;
    }
};