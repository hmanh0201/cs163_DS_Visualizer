#pragma once
#include <raylib.h>
#include "UI.h"
#include "theme.h"

class header
{
    public:
        header(int w, int h, const char *closeImg, const char *closeHoverImg, const char *themeImg, const char *themeHoverImg, const char *logoImg, const char *logoImgHover);
        ~header();
        void Draw();
        bool getState();
        bool getThemeState();
        bool getReturnMenu();
        
    private:
        int width;
        int height;
        bool state = 1;
        bool themeState = 1;
        bool returnMenu = 0;
        
        Texture2D logoTexture;
        Texture2D logoHoverTexture;
        button closeButton;
        button themeLightButton;
        button themeDarkButton;
        
        // Thêm biến để lưu Font chữ
        Font titleFont; 
};